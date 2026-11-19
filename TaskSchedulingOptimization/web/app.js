/* ================================================================
   Task Scheduling Optimization — app.js
   Vanilla ES modules, no libraries, no frameworks.
   ================================================================ */

/* ------------------------------------------------------------------ */
/*  API helpers                                                         */
/* ------------------------------------------------------------------ */
const API = {
  async get(path) {
    const r = await fetch(path);
    if (!r.ok) {
      const t = await r.text();
      throw new Error(`${r.status}: ${t}`);
    }
    return r.json();
  },
  async post(path, body) {
    const r = await fetch(path, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(body),
    });
    if (!r.ok) {
      const t = await r.text();
      throw new Error(`${r.status}: ${t}`);
    }
    return r.json();
  },
};

/* ------------------------------------------------------------------ */
/*  Jobs table                                                          */
/* ------------------------------------------------------------------ */
let jobRows = [];   // [{id, name, deadline, profit}, ...]

function getJobsFromTable() {
  return jobRows.slice();
}

function renderJobsTable() {
  const tbody = document.getElementById('jobs-tbody');
  const empty = document.getElementById('jobs-empty');
  tbody.innerHTML = '';
  if (jobRows.length === 0) {
    empty.style.display = '';
    return;
  }
  empty.style.display = 'none';

  jobRows.forEach((job, idx) => {
    const tr = document.createElement('tr');
    tr.dataset.idx = idx;
    tr.innerHTML = `
      <td class="id-cell num">${job.id}</td>
      <td><input type="text" value="${escHtml(job.name)}" aria-label="Job name" data-field="name" /></td>
      <td><input type="number" value="${job.deadline}" min="1" aria-label="Deadline" data-field="deadline" class="num" style="width:70px;text-align:right;" /></td>
      <td><input type="number" value="${job.profit}"   min="1" aria-label="Profit"   data-field="profit"   class="num" style="width:80px;text-align:right;" /></td>
      <td><button class="btn btn-danger btn-sm" data-remove="${idx}" aria-label="Remove job ${job.name}">×</button></td>
    `;
    // live edit
    tr.querySelectorAll('input').forEach(inp => {
      inp.addEventListener('input', () => {
        const field = inp.dataset.field;
        const val = field === 'name' ? inp.value : Number(inp.value);
        jobRows[idx][field] = val;
        validateRow(inp, field, val);
      });
    });
    tbody.appendChild(tr);
  });

  tbody.querySelectorAll('[data-remove]').forEach(btn => {
    btn.addEventListener('click', () => {
      jobRows.splice(Number(btn.dataset.remove), 1);
      renderJobsTable();
    });
  });
}

function validateRow(inp, field, val) {
  let ok = true;
  if (field === 'deadline' && (isNaN(val) || val < 1)) ok = false;
  if (field === 'profit'   && (isNaN(val) || val <= 0)) ok = false;
  inp.classList.toggle('invalid', !ok);
  return ok;
}

function validateAll() {
  if (jobRows.length === 0) return { ok: false, msg: 'Add at least one job.' };
  for (let i = 0; i < jobRows.length; i++) {
    const j = jobRows[i];
    if (!j.name || String(j.name).trim() === '') return { ok: false, msg: `Row ${i+1}: name is empty.` };
    if (isNaN(j.deadline) || j.deadline < 1)       return { ok: false, msg: `Row ${i+1}: deadline must be ≥ 1.` };
    if (isNaN(j.profit)   || j.profit <= 0)         return { ok: false, msg: `Row ${i+1}: profit must be > 0.` };
  }
  return { ok: true };
}

function showErr(id, msg) {
  const el = document.getElementById(id);
  if (!el) return;
  el.textContent = msg;
  el.classList.toggle('visible', !!msg);
}

function addBlankRow() {
  if (jobRows.length >= 20) { showErr('jobs-error', 'Maximum 20 jobs.'); return; }
  showErr('jobs-error', '');
  const nextId = jobRows.length ? Math.max(...jobRows.map(j => j.id)) + 1 : 1;
  jobRows.push({ id: nextId, name: `J${nextId}`, deadline: 2, profit: 50 });
  renderJobsTable();
}

function loadJobsData(data) {
  jobRows = data.map(j => ({
    id:       Number(j.id),
    name:     String(j.name),
    deadline: Number(j.deadline),
    profit:   Number(j.profit),
  }));
  renderJobsTable();
  showErr('jobs-error', '');
}

/* ------------------------------------------------------------------ */
/*  Greedy player state                                                 */
/* ------------------------------------------------------------------ */
let greedyData   = null;  // full API response
let playerStep   = -1;    // current step index (-1 = not started)
let playerTimer  = null;

function resetPlayer() {
  greedyData  = null;
  playerStep  = -1;
  if (playerTimer) { clearInterval(playerTimer); playerTimer = null; }
  setPlayerEnabled(false);
  document.getElementById('profit-counter').textContent = '0';
  document.getElementById('step-counter').textContent   = '— / —';
  document.getElementById('step-caption').textContent   = 'Run the greedy algorithm above to begin stepping through each decision.';
  document.getElementById('step-caption').className     = 'player-caption';
  document.getElementById('player-timetable').style.display = 'none';
  document.getElementById('player-tt-empty').style.display  = '';
  document.getElementById('step-scrubber').value        = 0;
  document.getElementById('step-scrubber').max          = 0;
}

function setPlayerEnabled(on) {
  ['btn-prev','btn-play','btn-next','step-scrubber'].forEach(id => {
    document.getElementById(id).disabled = !on;
  });
}

function applyGreedyData(data) {
  greedyData = data;
  playerStep = -1;
  const scrubber = document.getElementById('step-scrubber');
  scrubber.max   = data.steps.length - 1;
  scrubber.value = 0;
  setPlayerEnabled(true);
  document.getElementById('step-counter').textContent = `0 / ${data.steps.length}`;
  renderPlayerStep(0);
}

function renderPlayerStep(idx) {
  if (!greedyData) return;
  const steps  = greedyData.steps;
  if (idx < 0) idx = 0;
  if (idx >= steps.length) idx = steps.length - 1;
  playerStep = idx;

  const step = steps[idx];
  const scrubber = document.getElementById('step-scrubber');
  scrubber.value = idx;
  document.getElementById('step-counter').textContent = `${idx + 1} / ${steps.length}`;

  // Profit counter
  document.getElementById('profit-counter').textContent = step.running_profit;

  // Caption
  const caption = document.getElementById('step-caption');
  caption.textContent = step.caption;
  caption.className = 'player-caption' + (step.slot_assigned > 0 ? ' has-slot' : ' rejected');

  // Render player timetable at this step
  renderPlayerTimetable(idx);
}

function renderPlayerTimetable(upToStep) {
  const data  = greedyData;
  const steps = data.steps;

  // determine number of slots
  let numSlots = data.num_slots || 1;

  const wrap = document.getElementById('player-timetable');
  wrap.style.display = '';
  document.getElementById('player-tt-empty').style.display = 'none';

  // Build slot state up to and including upToStep
  const slotMap = {}; // slot index (1-based) -> step
  for (let i = 0; i <= upToStep; i++) {
    const s = steps[i];
    if (s.slot_assigned > 0) slotMap[s.slot_assigned] = s;
  }

  const colCount = numSlots + 1; // +1 for row label
  const grid = document.createElement('div');
  grid.className = 'timetable';
  grid.style.gridTemplateColumns = `auto repeat(${numSlots}, minmax(80px, 1fr))`;

  // Header row
  const rowLabelHead = document.createElement('div');
  rowLabelHead.className = 'tt-row-label';
  rowLabelHead.textContent = '';
  grid.appendChild(rowLabelHead);

  for (let s = 1; s <= numSlots; s++) {
    const cell = document.createElement('div');
    cell.className = 'tt-cell';
    const label = document.createElement('div');
    label.className = 'tt-slot-label';
    label.textContent = `T${s}`;
    cell.appendChild(label);
    grid.appendChild(cell);
  }

  // Schedule row
  const rowLabel = document.createElement('div');
  rowLabel.className = 'tt-row-label';
  rowLabel.textContent = 'Schedule';
  grid.appendChild(rowLabel);

  for (let s = 1; s <= numSlots; s++) {
    const cell = document.createElement('div');
    cell.className = 'tt-cell';
    if (slotMap[s]) {
      const step = slotMap[s];
      const tile = document.createElement('div');
      tile.className = 'tt-job-tile' +
        (step.slot_assigned === steps[upToStep].slot_assigned && upToStep >= 0 ? ' highlight animate-in' : '');
      tile.innerHTML = `
        <span class="tt-job-id">${step.job_id}</span>
        <span class="tt-job-name">${escHtml(step.job_name)}</span>
        <span class="tt-job-profit">+${step.profit}</span>
      `;
      cell.appendChild(tile);
    } else {
      cell.classList.add('tt-empty');
    }
    grid.appendChild(cell);
  }

  // Current job being considered (highlight it)
  const cur = steps[upToStep];
  const infoDiv = document.createElement('div');
  infoDiv.style.cssText = 'margin-top:.75rem;font-family:var(--ff-mono);font-size:.75rem;color:var(--muted);';
  infoDiv.textContent = `Considering: ${cur.job_name} — profit ${cur.profit}, deadline ${cur.deadline}`;
  if (cur.slot_assigned < 0) {
    infoDiv.style.color = 'var(--muted)';
    infoDiv.textContent += '  → rejected';
  }

  wrap.innerHTML = '';
  wrap.appendChild(grid);
  wrap.appendChild(infoDiv);
}

/* ------------------------------------------------------------------ */
/*  Final timetable (Section 03)                                       */
/* ------------------------------------------------------------------ */
function renderFinalTimetable(data, jobs, containerId) {
  const container = document.getElementById(containerId);
  if (!container) return;
  container.innerHTML = '';

  const numSlots = data.num_slots || 0;
  if (numSlots === 0) {
    container.innerHTML = '<div class="state-empty">No slots were allocated.</div>';
    return;
  }

  // Build job lookup by id
  const jobById = {};
  (jobs || []).forEach(j => { jobById[j.id] = j; });

  const grid = document.createElement('div');
  grid.className = 'timetable';
  grid.style.gridTemplateColumns = `auto repeat(${numSlots}, minmax(88px, 1fr))`;

  // Header
  const lh = document.createElement('div');
  lh.className = 'tt-row-label';
  lh.textContent = '';
  grid.appendChild(lh);
  for (let s = 1; s <= numSlots; s++) {
    const cell = document.createElement('div');
    cell.className = 'tt-cell';
    const lbl = document.createElement('div');
    lbl.className = 'tt-slot-label';
    lbl.textContent = `T${s}`;
    cell.appendChild(lbl);
    grid.appendChild(cell);
  }

  // Schedule row
  const lr = document.createElement('div');
  lr.className = 'tt-row-label';
  lr.textContent = 'Schedule';
  grid.appendChild(lr);

  for (let s = 1; s <= numSlots; s++) {
    const jobId = data.slots[s - 1];
    const cell = document.createElement('div');
    cell.className = 'tt-cell';
    if (jobId > 0) {
      const job = jobById[jobId] || { id: jobId, name: `J${jobId}`, profit: '?' };
      const tile = document.createElement('div');
      tile.className = 'tt-job-tile';
      tile.innerHTML = `
        <span class="tt-job-id">${job.id}</span>
        <span class="tt-job-name">${escHtml(job.name)}</span>
        <span class="tt-job-profit">+${job.profit}</span>
      `;
      cell.appendChild(tile);
    } else {
      cell.classList.add('tt-empty');
    }
    grid.appendChild(cell);
  }

  container.appendChild(grid);

  // Rejected jobs
  const rejected = (data.steps || []).filter(s => s.slot_assigned < 0);
  if (rejected.length) {
    const rejLabel = document.createElement('div');
    rejLabel.style.cssText = 'margin-top:1.5rem;font-family:var(--ff-mono);font-size:.75rem;color:var(--muted);letter-spacing:.06em;text-transform:uppercase;margin-bottom:.5rem;';
    rejLabel.textContent = 'Not scheduled';
    container.appendChild(rejLabel);

    const rejList = document.createElement('div');
    rejList.className = 'rejected-list';
    rejected.forEach(s => {
      const tile = document.createElement('div');
      tile.className = 'rejected-tile';
      tile.innerHTML = `${escHtml(s.job_name)}<span class="rejected-reason">no free slot by deadline ${s.deadline}</span>`;
      rejList.appendChild(tile);
    });
    container.appendChild(rejList);
  }
}

/* ------------------------------------------------------------------ */
/*  Verification (Section 04)                                          */
/* ------------------------------------------------------------------ */
function renderVerification(data, jobs) {
  const container = document.getElementById('verify-container');
  container.innerHTML = '';  // clears any previous result or state-empty div

  // Stamp
  const stampWrap = document.createElement('div');
  stampWrap.className = 'stamp-wrap';
  const stamp = document.createElement('div');
  stamp.className = 'stamp ' + (data.optimal ? 'stamp-optimal' : 'stamp-mismatch');
  stamp.textContent = data.optimal ? 'Verified Optimal' : 'Mismatch';
  stampWrap.appendChild(stamp);
  container.appendChild(stampWrap);

  // Side-by-side comparison
  const grid = document.createElement('div');
  grid.className = 'compare-grid mt-8';

  ['greedy', 'bruteforce'].forEach(key => {
    const result = data[key];
    const label  = key === 'greedy' ? 'Greedy' : 'Brute Force';
    const block  = document.createElement('div');
    block.className = 'compare-block';

    block.innerHTML = `
      <div class="compare-block-header">
        <span>${label}</span>
        <span class="tag ${data.optimal ? 'tag-optimal' : ''}">${data.optimal ? 'optimal' : key === 'bruteforce' ? 'reference' : 'sub-optimal'}</span>
      </div>
      <div class="compare-block-body">
        <div class="compare-stat">
          <span class="compare-stat-label">Total profit</span>
          <span class="compare-stat-value">${result.total_profit}</span>
        </div>
        <div class="compare-stat">
          <span class="compare-stat-label">Jobs scheduled</span>
          <span class="compare-stat-value">${result.jobs_done}</span>
        </div>
        <div class="compare-stat">
          <span class="compare-stat-label">Time slots used</span>
          <span class="compare-stat-value">${result.num_slots}</span>
        </div>
        <div class="compare-stat">
          <span class="compare-stat-label">Execution time</span>
          <span class="compare-stat-value">${result.time_ms.toFixed(3)} ms</span>
        </div>
      </div>
    `;

    // Mini timetable
    const ttDiv = document.createElement('div');
    ttDiv.style.cssText = 'padding:0 1rem 1rem;';
    const ttId = `cmp-tt-${key}`;
    ttDiv.id = ttId;
    block.appendChild(ttDiv);
    grid.appendChild(block);

    // Render after appending so IDs exist
    setTimeout(() => renderCompactTimetable(result, jobs, ttId), 0);
  });

  container.appendChild(grid);
}

function renderCompactTimetable(result, jobs, containerId) {
  const container = document.getElementById(containerId);
  if (!container) return;

  const numSlots = result.num_slots || 0;
  if (numSlots === 0) return;

  const jobById = {};
  (jobs || []).forEach(j => { jobById[j.id] = j; });

  const grid = document.createElement('div');
  grid.className = 'timetable';
  grid.style.gridTemplateColumns = `repeat(${numSlots}, minmax(60px, 1fr))`;
  grid.style.fontSize = '.75rem';

  for (let s = 1; s <= numSlots; s++) {
    const jobId = result.slots[s - 1];
    const cell  = document.createElement('div');
    cell.className = 'tt-cell';
    cell.style.minWidth = '60px';

    const lbl = document.createElement('div');
    lbl.className = 'tt-slot-label';
    lbl.textContent = `T${s}`;
    cell.appendChild(lbl);

    if (jobId > 0) {
      const job  = jobById[jobId] || { id: jobId, name: `J${jobId}`, profit: '?' };
      const tile = document.createElement('div');
      tile.className = 'tt-job-tile';
      tile.style.padding = '.25rem .5rem';
      tile.innerHTML = `<span class="tt-job-name" style="font-size:.75rem;">${escHtml(job.name)}</span>`;
      cell.appendChild(tile);
    } else {
      cell.classList.add('tt-empty');
      cell.style.minHeight = '40px';
    }
    grid.appendChild(cell);
  }
  container.appendChild(grid);
}

/* ------------------------------------------------------------------ */
/*  Performance chart — inline SVG                                     */
/* ------------------------------------------------------------------ */
function renderPerfChart(data) {
  const container = document.getElementById('perf-container');
  container.innerHTML = '';

  const W = 700, H = 320;
  const PAD = { top: 24, right: 100, bottom: 48, left: 72 };
  const IW  = W - PAD.left - PAD.right;
  const IH  = H - PAD.top  - PAD.bottom;

  const ns = data.map(d => d.n);
  const allTimes = data.flatMap(d => [d.greedy_ms, d.bruteforce_ms]).filter(v => v > 0);

  const minN  = Math.min(...ns);
  const maxN  = Math.max(...ns);
  const minT  = Math.min(...allTimes);
  const maxT  = Math.max(...allTimes);

  // Log scale helpers
  const logMin = Math.log10(Math.max(minT, 0.0001));
  const logMax = Math.log10(maxT * 2);

  function xOf(n) {
    return PAD.left + ((n - minN) / (maxN - minN)) * IW;
  }
  function yOf(t) {
    if (t <= 0) t = 0.0001;
    return PAD.top + IH - ((Math.log10(t) - logMin) / (logMax - logMin)) * IH;
  }

  function polyline(key, cssClass) {
    const pts = data.map(d => `${xOf(d.n).toFixed(1)},${yOf(d[key]).toFixed(1)}`).join(' ');
    return `<polyline class="${cssClass}" points="${pts}" />`;
  }

  // Gridlines (log scale, horizontal)
  let gridlines = '';
  for (let e = Math.floor(logMin); e <= Math.ceil(logMax); e++) {
    const y = yOf(Math.pow(10, e));
    if (y < PAD.top || y > PAD.top + IH) continue;
    const val = Math.pow(10, e);
    const lbl = val < 0.01 ? val.toExponential(0) : val >= 1 ? `${val.toFixed(0)}ms` : `${(val*1000).toFixed(0)}µs`;
    gridlines += `<line class="chart-gridline" x1="${PAD.left}" y1="${y.toFixed(1)}" x2="${PAD.left+IW}" y2="${y.toFixed(1)}" />`;
    gridlines += `<text class="chart-label" x="${PAD.left-6}" y="${(y+4).toFixed(1)}" text-anchor="end">${lbl}</text>`;
  }

  // X-axis labels
  let xlabels = '';
  ns.forEach(n => {
    const x = xOf(n);
    xlabels += `<text class="chart-label" x="${x.toFixed(1)}" y="${(PAD.top+IH+18).toFixed(1)}" text-anchor="middle">${n}</text>`;
  });

  // End labels
  const lastD = data[data.length - 1];
  const gEndX = xOf(lastD.n) + 6, gEndY = yOf(lastD.greedy_ms);
  const bEndX = xOf(lastD.n) + 6, bEndY = yOf(lastD.bruteforce_ms);

  // Dash-draw animation via stroke-dashoffset
  const totalLen = IW * 1.5; // generous estimate

  const svg = `
<svg class="chart-svg" viewBox="0 0 ${W} ${H}" xmlns="http://www.w3.org/2000/svg"
     role="img" aria-label="Performance comparison chart" style="max-width:100%;height:auto;">
  <title>Greedy vs Brute Force — execution time vs n</title>

  <!-- Grid -->
  ${gridlines}

  <!-- Axes -->
  <line class="chart-axis" x1="${PAD.left}" y1="${PAD.top}" x2="${PAD.left}" y2="${PAD.top+IH}" />
  <line class="chart-axis" x1="${PAD.left}" y1="${PAD.top+IH}" x2="${PAD.left+IW}" y2="${PAD.top+IH}" />

  <!-- X labels -->
  ${xlabels}
  <text class="chart-label" x="${(PAD.left+IW/2).toFixed(1)}" y="${H-6}" text-anchor="middle">Number of Jobs (n)</text>
  <text class="chart-label" x="12" y="${(PAD.top+IH/2).toFixed(1)}" text-anchor="middle"
        transform="rotate(-90,12,${(PAD.top+IH/2).toFixed(1)})">Time (log scale)</text>

  <!-- Lines with draw-in animation -->
  <style>
    .line-anim {
      stroke-dasharray: ${totalLen};
      stroke-dashoffset: ${totalLen};
      animation: drawLine 1.2s ease-out forwards;
    }
    @keyframes drawLine { to { stroke-dashoffset: 0; } }
    @media (prefers-reduced-motion: reduce) { .line-anim { animation: none; stroke-dashoffset: 0; } }
    .line-brute-anim { animation-delay: 0.3s; }
  </style>

  <polyline class="line-greedy line-anim" points="${data.map(d=>`${xOf(d.n).toFixed(1)},${yOf(d.greedy_ms).toFixed(1)}`).join(' ')}" />
  <polyline class="line-brute line-anim line-brute-anim" points="${data.map(d=>`${xOf(d.n).toFixed(1)},${yOf(d.bruteforce_ms).toFixed(1)}`).join(' ')}" />

  <!-- End labels -->
  <text class="chart-label" x="${(gEndX).toFixed(1)}" y="${(gEndY+4).toFixed(1)}" fill="var(--ink)" font-weight="500">Greedy</text>
  <text class="chart-label" x="${(bEndX).toFixed(1)}" y="${(bEndY+4).toFixed(1)}" fill="var(--vermilion)" font-weight="500">Brute force</text>
</svg>`;

  const wrap = document.createElement('div');
  wrap.className = 'chart-wrap';
  wrap.innerHTML = svg;
  container.appendChild(wrap);

  document.getElementById('perf-takeaway').style.display = '';
  document.getElementById('perf-empty').style.display    = 'none';
}

/* ------------------------------------------------------------------ */
/*  Utility                                                             */
/* ------------------------------------------------------------------ */
function escHtml(str) {
  return String(str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

function showLoading(id, msg) {
  const el = document.getElementById(id);
  if (el) el.innerHTML = `<div class="state-loading">${msg}</div>`;
}

/* ------------------------------------------------------------------ */
/*  Wire up buttons                                                     */
/* ------------------------------------------------------------------ */
document.getElementById('btn-load-sample').addEventListener('click', async () => {
  showErr('jobs-error', '');
  try {
    const data = await API.get('/api/sample');
    loadJobsData(data);
  } catch (e) {
    showErr('jobs-error', 'Could not load sample: ' + e.message);
  }
});

document.getElementById('btn-random').addEventListener('click', async () => {
  showErr('jobs-error', '');
  try {
    const data = await API.post('/api/jobs/random?n=8', {});
    loadJobsData(data);
  } catch (e) {
    showErr('jobs-error', 'Could not generate random jobs: ' + e.message);
  }
});

document.getElementById('btn-add-row').addEventListener('click', addBlankRow);

document.getElementById('btn-run-greedy').addEventListener('click', async () => {
  const v = validateAll();
  if (!v.ok) { showErr('jobs-error', v.msg); return; }
  showErr('jobs-error', '');
  showErr('greedy-error', '');
  resetPlayer();

  const jobs = getJobsFromTable();

  // Show loading state in Section 03
  showLoading('timetable-container', 'Running greedy algorithm');

  try {
    const data = await API.post('/api/greedy', jobs);
    applyGreedyData(data);

    // Also render final timetable in s03
    const container = document.getElementById('timetable-container');
    container.innerHTML = '';
    // tt-empty is inside timetable-container, so it's already gone — no need to hide it
    renderFinalTimetable(data, jobs, 'timetable-container');

    // Scroll to s02
    document.getElementById('s02').scrollIntoView({ behavior: 'smooth' });
  } catch (e) {
    showErr('greedy-error', 'Error: ' + e.message);
    document.getElementById('timetable-container').innerHTML =
      '<div class="state-error">Failed to run algorithm. See error above.</div>';
  }
});

document.getElementById('btn-run-compare').addEventListener('click', async () => {
  const v = validateAll();
  if (!v.ok) { showErr('jobs-error', v.msg); return; }
  showErr('jobs-error', '');

  const jobs = getJobsFromTable();
  const container = document.getElementById('verify-container');
  container.innerHTML = '<div class="state-loading">Running verification (brute force may take a moment)</div>';

  try {
    const data = await API.post('/api/compare', jobs);
    renderVerification(data, jobs);
    // verify-empty is inside verify-container, already cleared by renderVerification
    document.getElementById('s04').scrollIntoView({ behavior: 'smooth' });
  } catch (e) {
    container.innerHTML = `<div class="state-error">Error: ${escHtml(e.message)}</div>`;
  }
});

/* Player controls */
document.getElementById('btn-prev').addEventListener('click', () => {
  if (greedyData) renderPlayerStep(playerStep - 1);
});
document.getElementById('btn-next').addEventListener('click', () => {
  if (greedyData) renderPlayerStep(playerStep + 1);
});
document.getElementById('btn-play').addEventListener('click', () => {
  if (!greedyData) return;
  const btn = document.getElementById('btn-play');
  if (playerTimer) {
    clearInterval(playerTimer);
    playerTimer = null;
    btn.textContent = 'Play';
    return;
  }
  btn.textContent = 'Pause';
  if (playerStep >= greedyData.steps.length - 1) renderPlayerStep(0);
  playerTimer = setInterval(() => {
    renderPlayerStep(playerStep + 1);
    if (playerStep >= greedyData.steps.length - 1) {
      clearInterval(playerTimer);
      playerTimer = null;
      btn.textContent = 'Play';
    }
  }, 800);
});
document.getElementById('step-scrubber').addEventListener('input', e => {
  if (greedyData) renderPlayerStep(Number(e.target.value));
});

/* Keyboard: arrow keys on scrubber */
document.getElementById('step-scrubber').addEventListener('keydown', e => {
  if (!greedyData) return;
  if (e.key === 'ArrowLeft')  { e.preventDefault(); renderPlayerStep(playerStep - 1); }
  if (e.key === 'ArrowRight') { e.preventDefault(); renderPlayerStep(playerStep + 1); }
});

/* Performance */
document.getElementById('btn-run-perf').addEventListener('click', async () => {
  const container = document.getElementById('perf-container');
  container.innerHTML = '<div class="state-loading">Timing algorithms for n = 5, 8, 10, 12, 15, 18</div>';
  try {
    const data = await API.get('/api/performance');
    renderPerfChart(data);
  } catch (e) {
    container.innerHTML = `<div class="state-error">Error: ${escHtml(e.message)}</div>`;
  }
});

/* ------------------------------------------------------------------ */
/*  Initial state                                                       */
/* ------------------------------------------------------------------ */
renderJobsTable();  // show empty state
