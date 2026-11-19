/*
 * Module: HTTP Server
 * Description: Minimal HTTP/1.0 server in pure C using POSIX sockets.
 *   Serves static files from web/ and JSON API endpoints.
 *   Cross-platform: #ifdef _WIN32 for Winsock, otherwise POSIX.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef int socklen_t;
  #define CLOSE_SOCKET(s) closesocket(s)
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #define CLOSE_SOCKET(s) close(s)
#endif

#include "../include/server.h"
#include "../include/job.h"
#include "../include/greedy.h"
#include "../include/bruteforce.h"

/* ------------------------------------------------------------------ */
/*  Helpers                                                             */
/* ------------------------------------------------------------------ */

#define BUF_SIZE    (1 << 20)  /* 1 MB response buffer */
#define BODY_SIZE   (1 << 16)  /* 64 KB request body   */
#define PATH_MAX_LEN 256

static void send_all(int fd, const char *buf, int len) {
    int sent = 0;
    while (sent < len) {
        int n;
#ifdef _WIN32
        n = send(fd, buf + sent, len - sent, 0);
#else
        n = (int)write(fd, buf + sent, len - sent);
#endif
        if (n <= 0) break;
        sent += n;
    }
}

static void send_response(int fd, int status, const char *content_type,
                           const char *body, int body_len) {
    char header[512];
    const char *status_text = "OK";
    if (status == 400) status_text = "Bad Request";
    else if (status == 404) status_text = "Not Found";
    else if (status == 405) status_text = "Method Not Allowed";

    int hlen = snprintf(header, sizeof(header),
        "HTTP/1.0 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, status_text, content_type, body_len);
    send_all(fd, header, hlen);
    if (body_len > 0) send_all(fd, body, body_len);
}

static void send_json(int fd, int status, const char *json) {
    send_response(fd, status, "application/json", json, (int)strlen(json));
}

/* ------------------------------------------------------------------ */
/*  Minimal JSON parser for job arrays from POST body                   */
/*  Format: [{"id":1,"name":"J1","deadline":2,"profit":100}, ...]      */
/* ------------------------------------------------------------------ */

static int parse_int_field(const char *body, const char *key, int *out) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char *p = strstr(body, search);
    if (!p) return 0;
    p += strlen(search);
    while (*p == ' ' || *p == '\t') p++;
    *out = atoi(p);
    return 1;
}

static int parse_string_field(const char *body, const char *key, char *out, int maxlen) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char *p = strstr(body, search);
    if (!p) return 0;
    p += strlen(search);
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '"') return 0;
    p++;
    int i = 0;
    while (*p && *p != '"' && i < maxlen - 1) out[i++] = *p++;
    out[i] = '\0';
    return 1;
}

/* Parse jobs array from JSON body. Returns count parsed. */
static int parse_jobs_json(const char *body, Job *jobs) {
    int count = 0;
    const char *p = body;
    while (count < MAX_JOBS) {
        /* find next { */
        p = strchr(p, '{');
        if (!p) break;
        /* find matching } */
        const char *end = strchr(p, '}');
        if (!end) break;
        int len = (int)(end - p) + 1;
        char obj[256];
        if (len >= (int)sizeof(obj)) { p = end + 1; continue; }
        memcpy(obj, p, len);
        obj[len] = '\0';

        int id = 0, deadline = 0, profit = 0;
        char name[30] = "J?";
        parse_int_field(obj, "id", &id);
        parse_string_field(obj, "name", name, 30);
        parse_int_field(obj, "deadline", &deadline);
        parse_int_field(obj, "profit", &profit);

        if (id > 0 && deadline >= 1 && profit > 0) {
            jobs[count].id       = id;
            strncpy(jobs[count].name, name, 29);
            jobs[count].name[29] = '\0';
            jobs[count].deadline = deadline;
            jobs[count].profit   = profit;
            count++;
        }
        p = end + 1;
    }
    return count;
}

/* ------------------------------------------------------------------ */
/*  JSON builders                                                       */
/* ------------------------------------------------------------------ */

static int jobs_to_json(const Job *jobs, int count, char *buf, int maxlen) {
    int pos = 0;
    pos += snprintf(buf + pos, maxlen - pos, "[");
    for (int i = 0; i < count; i++) {
        if (i) pos += snprintf(buf + pos, maxlen - pos, ",");
        pos += snprintf(buf + pos, maxlen - pos,
            "{\"id\":%d,\"name\":\"%s\",\"deadline\":%d,\"profit\":%d}",
            jobs[i].id, jobs[i].name, jobs[i].deadline, jobs[i].profit);
    }
    pos += snprintf(buf + pos, maxlen - pos, "]");
    return pos;
}

/* Build schedule JSON + steps array.
   steps: for each job in profit-sorted order, we record:
     job, slot_assigned (or -1), reason */
static int schedule_with_steps_to_json(
        const Job *jobs, int count,
        const Schedule *sched,
        char *buf, int maxlen) {

    /* Rebuild the step log by re-running greedy (same logic) */
    /* Sort a copy by profit descending */
    Job sorted[MAX_JOBS];
    int sc = count < MAX_JOBS ? count : MAX_JOBS;
    for (int i = 0; i < sc; i++) sorted[i] = jobs[i];
    /* bubble sort */
    for (int i = 0; i < sc - 1; i++)
        for (int j = 0; j < sc - i - 1; j++)
            if (sorted[j].profit < sorted[j+1].profit) {
                Job t = sorted[j]; sorted[j] = sorted[j+1]; sorted[j+1] = t;
            }

    int max_dead = 0;
    for (int i = 0; i < sc; i++) if (sorted[i].deadline > max_dead) max_dead = sorted[i].deadline;
    if (max_dead > MAX_SLOTS) max_dead = MAX_SLOTS;

    int slot_track[MAX_SLOTS];
    for (int i = 0; i < MAX_SLOTS; i++) slot_track[i] = -1;

    int pos = 0;
    pos += snprintf(buf + pos, maxlen - pos, "{");
    pos += snprintf(buf + pos, maxlen - pos, "\"total_profit\":%d,", sched->total_profit);
    pos += snprintf(buf + pos, maxlen - pos, "\"jobs_done\":%d,", sched->jobs_done);
    pos += snprintf(buf + pos, maxlen - pos, "\"num_slots\":%d,", sched->num_slots);

    /* slots array */
    pos += snprintf(buf + pos, maxlen - pos, "\"slots\":[");
    for (int i = 0; i < sched->num_slots; i++) {
        if (i) pos += snprintf(buf + pos, maxlen - pos, ",");
        pos += snprintf(buf + pos, maxlen - pos, "%d", sched->slots[i]);
    }
    pos += snprintf(buf + pos, maxlen - pos, "],");

    /* steps array */
    int running_profit = 0;
    pos += snprintf(buf + pos, maxlen - pos, "\"steps\":[");
    for (int i = 0; i < sc; i++) {
        if (i) pos += snprintf(buf + pos, maxlen - pos, ",");
        int d = sorted[i].deadline;
        if (d > max_dead) d = max_dead;
        int orig_d = d;
        while (d > 0 && slot_track[d-1] != -1) d--;
        int assigned = -1;
        char reason[64] = "";
        if (d > 0) {
            slot_track[d-1] = sorted[i].id;
            assigned = d;
            running_profit += sorted[i].profit;
            snprintf(reason, sizeof(reason),
                "%s (profit %d, deadline %d) takes slot %d",
                sorted[i].name, sorted[i].profit, sorted[i].deadline, d);
        } else {
            snprintf(reason, sizeof(reason),
                "%s (profit %d, deadline %d) rejected \xe2\x80\x93 all slots up to %d full",
                sorted[i].name, sorted[i].profit, sorted[i].deadline, orig_d);
        }
        pos += snprintf(buf + pos, maxlen - pos,
            "{\"job_id\":%d,\"job_name\":\"%s\",\"profit\":%d,\"deadline\":%d,"
            "\"slot_assigned\":%d,\"running_profit\":%d,\"caption\":\"%s\"}",
            sorted[i].id, sorted[i].name, sorted[i].profit, sorted[i].deadline,
            assigned, running_profit, reason);
    }
    pos += snprintf(buf + pos, maxlen - pos, "]}");
    return pos;
}

/* ------------------------------------------------------------------ */
/*  API handlers                                                        */
/* ------------------------------------------------------------------ */

/* GET /api/sample */
static void handle_sample(int fd) {
    Job jobs[MAX_JOBS];
    int count = 0;
    /* Try a few known paths for jobs.txt */
    const char *paths[] = {
        "data/jobs.txt",
        "../data/jobs.txt",
        "TaskSchedulingOptimization/data/jobs.txt"
    };
    for (int i = 0; i < 3; i++) {
        if (load_jobs_from_file(paths[i], jobs, &count) == 0 && count > 0) break;
    }
    if (count == 0) {
        send_json(fd, 404, "{\"error\":\"jobs.txt not found\"}");
        return;
    }
    char *buf = (char*)malloc(BUF_SIZE);
    if (!buf) { send_json(fd, 500, "{\"error\":\"oom\"}"); return; }
    int len = jobs_to_json(jobs, count, buf, BUF_SIZE);
    send_response(fd, 200, "application/json", buf, len);
    free(buf);
}

/* POST /api/jobs/random?n=8 */
static void handle_random(int fd, const char *query) {
    int n = 8;
    const char *nq = strstr(query, "n=");
    if (nq) n = atoi(nq + 2);
    if (n < 1) n = 1;
    if (n > MAX_JOBS) n = MAX_JOBS;

    Job jobs[MAX_JOBS];
    generate_random_jobs(jobs, n);

    char *buf = (char*)malloc(BUF_SIZE);
    if (!buf) { send_json(fd, 500, "{\"error\":\"oom\"}"); return; }
    int len = jobs_to_json(jobs, n, buf, BUF_SIZE);
    send_response(fd, 200, "application/json", buf, len);
    free(buf);
}

/* POST /api/greedy  body: JSON jobs array */
static void handle_greedy(int fd, const char *body) {
    Job jobs[MAX_JOBS];
    int count = parse_jobs_json(body, jobs);
    if (count == 0) {
        send_json(fd, 400, "{\"error\":\"no valid jobs in body\"}");
        return;
    }
    Schedule sched = greedy_schedule(jobs, count);

    char *buf = (char*)malloc(BUF_SIZE);
    if (!buf) { send_json(fd, 500, "{\"error\":\"oom\"}"); return; }
    int len = schedule_with_steps_to_json(jobs, count, &sched, buf, BUF_SIZE);
    send_response(fd, 200, "application/json", buf, len);
    free(buf);
}

/* POST /api/bruteforce  body: JSON jobs array */
static void handle_bruteforce(int fd, const char *body) {
    Job jobs[MAX_JOBS];
    int count = parse_jobs_json(body, jobs);
    if (count == 0) {
        send_json(fd, 400, "{\"error\":\"no valid jobs in body\"}");
        return;
    }

    struct timespec t0, t1;
#ifdef _WIN32
    clock_t c0 = clock();
    Schedule sched = bruteforce_schedule(jobs, count);
    clock_t c1 = clock();
    double ms = (double)(c1 - c0) * 1000.0 / CLOCKS_PER_SEC;
#else
    clock_gettime(CLOCK_MONOTONIC, &t0);
    Schedule sched = bruteforce_schedule(jobs, count);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double ms = (t1.tv_sec - t0.tv_sec) * 1000.0 +
                (t1.tv_nsec - t0.tv_nsec) / 1e6;
#endif

    char *buf = (char*)malloc(BUF_SIZE);
    if (!buf) { send_json(fd, 500, "{\"error\":\"oom\"}"); return; }
    int pos = 0;
    pos += snprintf(buf + pos, BUF_SIZE - pos, "{");
    pos += snprintf(buf + pos, BUF_SIZE - pos, "\"total_profit\":%d,", sched.total_profit);
    pos += snprintf(buf + pos, BUF_SIZE - pos, "\"jobs_done\":%d,", sched.jobs_done);
    pos += snprintf(buf + pos, BUF_SIZE - pos, "\"num_slots\":%d,", sched.num_slots);
    pos += snprintf(buf + pos, BUF_SIZE - pos, "\"time_ms\":%.3f,", ms);
    pos += snprintf(buf + pos, BUF_SIZE - pos, "\"slots\":[");
    for (int i = 0; i < sched.num_slots; i++) {
        if (i) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos, "%d", sched.slots[i]);
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]}");
    send_response(fd, 200, "application/json", buf, pos);
    free(buf);
}

/* POST /api/compare  body: JSON jobs array */
static void handle_compare(int fd, const char *body) {
    Job jobs[MAX_JOBS];
    int count = parse_jobs_json(body, jobs);
    if (count == 0) {
        send_json(fd, 400, "{\"error\":\"no valid jobs in body\"}");
        return;
    }

    struct timespec t0, t1, t2, t3;
#ifdef _WIN32
    clock_t c0 = clock();
    Schedule gsched = greedy_schedule(jobs, count);
    clock_t c1 = clock();
    Schedule bsched = bruteforce_schedule(jobs, count);
    clock_t c2 = clock();
    double gms = (double)(c1 - c0) * 1000.0 / CLOCKS_PER_SEC;
    double bms = (double)(c2 - c1) * 1000.0 / CLOCKS_PER_SEC;
#else
    clock_gettime(CLOCK_MONOTONIC, &t0);
    Schedule gsched = greedy_schedule(jobs, count);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    Schedule bsched = bruteforce_schedule(jobs, count);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    (void)t3;
    double gms = (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1e6;
    double bms = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_nsec - t1.tv_nsec) / 1e6;
#endif

    int optimal = (gsched.total_profit == bsched.total_profit);

    char *buf = (char*)malloc(BUF_SIZE);
    if (!buf) { send_json(fd, 500, "{\"error\":\"oom\"}"); return; }
    int pos = 0;
    pos += snprintf(buf + pos, BUF_SIZE - pos, "{");
    pos += snprintf(buf + pos, BUF_SIZE - pos, "\"optimal\":%s,", optimal ? "true" : "false");

    /* greedy block */
    pos += snprintf(buf + pos, BUF_SIZE - pos, "\"greedy\":{");
    pos += snprintf(buf + pos, BUF_SIZE - pos, "\"total_profit\":%d,\"jobs_done\":%d,\"num_slots\":%d,\"time_ms\":%.3f,\"slots\":[",
        gsched.total_profit, gsched.jobs_done, gsched.num_slots, gms);
    for (int i = 0; i < gsched.num_slots; i++) {
        if (i) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos, "%d", gsched.slots[i]);
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]},");

    /* bruteforce block */
    pos += snprintf(buf + pos, BUF_SIZE - pos, "\"bruteforce\":{");
    pos += snprintf(buf + pos, BUF_SIZE - pos, "\"total_profit\":%d,\"jobs_done\":%d,\"num_slots\":%d,\"time_ms\":%.3f,\"slots\":[",
        bsched.total_profit, bsched.jobs_done, bsched.num_slots, bms);
    for (int i = 0; i < bsched.num_slots; i++) {
        if (i) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos, "%d", bsched.slots[i]);
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]}");
    pos += snprintf(buf + pos, BUF_SIZE - pos, "}");

    send_response(fd, 200, "application/json", buf, pos);
    free(buf);
}

/* GET /api/performance  — timings for n = 5, 8, 10, 12, 15, 18 */
static void handle_performance(int fd) {
    int sizes[] = {5, 8, 10, 12, 15, 18};
    int ns = 6;
    char *buf = (char*)malloc(BUF_SIZE);
    if (!buf) { send_json(fd, 500, "{\"error\":\"oom\"}"); return; }
    int pos = 0;
    pos += snprintf(buf + pos, BUF_SIZE - pos, "[");
    for (int si = 0; si < ns; si++) {
        int n = sizes[si];
        Job jobs[MAX_JOBS];
        generate_random_jobs(jobs, n);

        struct timespec ta, tb, tc;
#ifdef _WIN32
        clock_t ca = clock();
        greedy_schedule(jobs, n);
        clock_t cb = clock();
        bruteforce_schedule(jobs, n);
        clock_t cc = clock();
        double gms = (double)(cb - ca) * 1000.0 / CLOCKS_PER_SEC;
        double bms = (double)(cc - cb) * 1000.0 / CLOCKS_PER_SEC;
#else
        clock_gettime(CLOCK_MONOTONIC, &ta);
        greedy_schedule(jobs, n);
        clock_gettime(CLOCK_MONOTONIC, &tb);
        bruteforce_schedule(jobs, n);
        clock_gettime(CLOCK_MONOTONIC, &tc);
        double gms = (tb.tv_sec - ta.tv_sec) * 1000.0 + (tb.tv_nsec - ta.tv_nsec) / 1e6;
        double bms = (tc.tv_sec - tb.tv_sec) * 1000.0 + (tc.tv_nsec - tb.tv_nsec) / 1e6;
#endif
        /* Ensure a minimum measurable value so the log chart doesn't break */
        if (gms < 0.001) gms = 0.001;
        if (bms < 0.001) bms = 0.001;

        if (si) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos,
            "{\"n\":%d,\"greedy_ms\":%.4f,\"bruteforce_ms\":%.4f}", n, gms, bms);
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]");
    send_response(fd, 200, "application/json", buf, pos);
    free(buf);
}

/* ------------------------------------------------------------------ */
/*  Static file serving from web/                                       */
/* ------------------------------------------------------------------ */

static const char *mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (strcmp(ext, ".html") == 0) return "text/html; charset=utf-8";
    if (strcmp(ext, ".css")  == 0) return "text/css; charset=utf-8";
    if (strcmp(ext, ".js")   == 0) return "application/javascript; charset=utf-8";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".svg")  == 0) return "image/svg+xml";
    if (strcmp(ext, ".ico")  == 0) return "image/x-icon";
    if (strcmp(ext, ".png")  == 0) return "image/png";
    return "application/octet-stream";
}

/* Search for the web/ directory relative to the binary or cwd */
static int find_web_file(const char *url_path, char *out, int outlen) {
    /* Sanitize: strip query string */
    char clean[PATH_MAX_LEN];
    strncpy(clean, url_path, PATH_MAX_LEN - 1);
    clean[PATH_MAX_LEN-1] = '\0';
    char *q = strchr(clean, '?');
    if (q) *q = '\0';

    /* Default to index.html */
    if (strcmp(clean, "/") == 0) strcpy(clean, "/index.html");

    /* Prevent path traversal */
    if (strstr(clean, "..")) return 0;

    const char *bases[] = { "web", "../web", "TaskSchedulingOptimization/web" };
    for (int i = 0; i < 3; i++) {
        snprintf(out, outlen, "%s%s", bases[i], clean);
        FILE *f = fopen(out, "rb");
        if (f) { fclose(f); return 1; }
    }
    return 0;
}

static void serve_static(int fd, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        send_json(fd, 404, "{\"error\":\"file not found\"}");
        return;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    char *body = (char*)malloc(sz + 1);
    if (!body) { fclose(f); send_json(fd, 500, "{\"error\":\"oom\"}"); return; }
    fread(body, 1, sz, f);
    fclose(f);
    send_response(fd, 200, mime_type(path), body, (int)sz);
    free(body);
}

/* ------------------------------------------------------------------ */
/*  Request parser & dispatcher                                         */
/* ------------------------------------------------------------------ */

static void handle_client(int fd) {
    char *raw = (char*)malloc(BODY_SIZE);
    if (!raw) { CLOSE_SOCKET(fd); return; }

    int total = 0;
    while (total < BODY_SIZE - 1) {
        int n;
#ifdef _WIN32
        n = recv(fd, raw + total, BODY_SIZE - 1 - total, 0);
#else
        n = (int)read(fd, raw + total, BODY_SIZE - 1 - total);
#endif
        if (n <= 0) break;
        total += n;
        raw[total] = '\0';
        /* Stop once headers + body are complete (simple heuristic) */
        if (strstr(raw, "\r\n\r\n")) {
            /* Check Content-Length */
            const char *cl = strstr(raw, "Content-Length:");
            if (!cl) cl = strstr(raw, "content-length:");
            if (cl) {
                int clen = atoi(cl + 15);
                const char *body_start = strstr(raw, "\r\n\r\n");
                if (body_start) {
                    int body_got = total - (int)(body_start + 4 - raw);
                    if (body_got >= clen) break;
                }
            } else {
                break;
            }
        }
    }
    raw[total] = '\0';

    /* Parse first line */
    char method[8] = "", url[PATH_MAX_LEN] = "", proto[16] = "";
    sscanf(raw, "%7s %255s %15s", method, url, proto);

    /* Separate path from query */
    char path[PATH_MAX_LEN] = "", query[PATH_MAX_LEN] = "";
    char *qm = strchr(url, '?');
    if (qm) {
        int plen = (int)(qm - url);
        if (plen >= PATH_MAX_LEN) plen = PATH_MAX_LEN - 1;
        strncpy(path, url, plen); path[plen] = '\0';
        strncpy(query, qm + 1, PATH_MAX_LEN - 1);
    } else {
        strncpy(path, url, PATH_MAX_LEN - 1);
    }

    /* Find request body */
    const char *body = "";
    const char *sep = strstr(raw, "\r\n\r\n");
    if (sep) body = sep + 4;

    /* Dispatch */
    if (strcmp(path, "/api/sample") == 0 && strcmp(method, "GET") == 0) {
        handle_sample(fd);
    } else if (strcmp(path, "/api/jobs/random") == 0) {
        handle_random(fd, query);
    } else if (strcmp(path, "/api/greedy") == 0 && strcmp(method, "POST") == 0) {
        handle_greedy(fd, body);
    } else if (strcmp(path, "/api/bruteforce") == 0 && strcmp(method, "POST") == 0) {
        handle_bruteforce(fd, body);
    } else if (strcmp(path, "/api/compare") == 0 && strcmp(method, "POST") == 0) {
        handle_compare(fd, body);
    } else if (strcmp(path, "/api/performance") == 0 && strcmp(method, "GET") == 0) {
        handle_performance(fd);
    } else {
        /* Static file */
        char filepath[PATH_MAX_LEN * 2];
        if (find_web_file(path[0] == '/' ? path : "/", filepath, sizeof(filepath))) {
            serve_static(fd, filepath);
        } else {
            /* 404 with helpful HTML */
            const char *body404 =
                "<!DOCTYPE html><html><head><title>404</title></head><body>"
                "<h1>404 Not Found</h1><p><a href='/'>Go to app</a></p>"
                "</body></html>";
            send_response(fd, 404, "text/html", body404, (int)strlen(body404));
        }
    }

    free(raw);
}

/* ------------------------------------------------------------------ */
/*  Main server loop                                                    */
/* ------------------------------------------------------------------ */

void server_start(int port) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int srv;
    srv = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return; }

    int opt = 1;
#ifdef _WIN32
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((unsigned short)port);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        CLOSE_SOCKET(srv);
        return;
    }
    if (listen(srv, 16) < 0) {
        perror("listen");
        CLOSE_SOCKET(srv);
        return;
    }

    printf("\n  Task Scheduling Optimization — Web UI\n");
    printf("  Server running at: http://localhost:%d\n", port);
    printf("  Press Ctrl-C to stop.\n\n");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t clen = sizeof(client_addr);
        int cfd = (int)accept(srv, (struct sockaddr*)&client_addr, &clen);
        if (cfd < 0) continue;
        handle_client(cfd);
        CLOSE_SOCKET(cfd);
    }

#ifdef _WIN32
    WSACleanup();
#endif
}
