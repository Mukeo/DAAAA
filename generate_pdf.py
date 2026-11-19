import os
import sys
from PIL import Image
from reportlab.lib.pagesizes import letter, landscape
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Image as RLImage, PageBreak, Table, TableStyle
from reportlab.pdfgen import canvas

SCREENSHOTS_DIR = "/Users/mukro/.gemini/antigravity/brain/7efbf533-af69-414b-ae98-92c387aa10a7/screenshots"
OUTPUT_PDF = "/Users/mukro/Downloads/DAAA/Task_Scheduling_Optimization_Screenshots.pdf"
BACKUP_PDF = "/Users/mukro/.gemini/antigravity/brain/7efbf533-af69-414b-ae98-92c387aa10a7/Task_Scheduling_Optimization_Screenshots.pdf"

class NumberedCanvas(canvas.Canvas):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_number(num_pages)
            canvas.Canvas.showPage(self)
        canvas.Canvas.save(self)

    def draw_page_number(self, page_count):
        if self._pageNumber == 1:
            return  # Skip cover page
        self.saveState()
        self.setFont("Helvetica", 9)
        self.setFillColor(colors.HexColor("#6B665B"))
        # Top rule
        self.setStrokeColor(colors.HexColor("#D8D1C0"))
        self.setLineWidth(0.5)
        self.line(40, 11 * 72 - 30, 8.5 * 72 - 40, 11 * 72 - 30)
        # Top title
        self.drawString(40, 11 * 72 - 25, "Task Scheduling Optimization — UI Walkthrough")
        
        # Bottom rule & page number
        self.line(40, 35, 8.5 * 72 - 40, 35)
        self.drawRightString(8.5 * 72 - 40, 22, f"Page {self._pageNumber} of {page_count}")
        self.drawString(40, 22, "C99 Backend • Pure Socket HTTP Server (Port 9090) • Timetable Board Web UI")
        self.restoreState()

def build_pdf():
    doc = SimpleDocTemplate(
        OUTPUT_PDF,
        pagesize=letter,
        leftMargin=40,
        rightMargin=40,
        topMargin=45,
        bottomMargin=45
    )
    
    styles = getSampleStyleSheet()
    
    title_style = ParagraphStyle(
        'CoverTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=28,
        leading=34,
        textColor=colors.HexColor("#1B1A17"),
        spaceAfter=10
    )
    
    subtitle_style = ParagraphStyle(
        'CoverSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=13,
        leading=18,
        textColor=colors.HexColor("#6B665B"),
        spaceAfter=25
    )
    
    section_tag_style = ParagraphStyle(
        'SectionTag',
        parent=styles['Normal'],
        fontName='Courier-Bold',
        fontSize=10,
        leading=12,
        textColor=colors.HexColor("#D9401F"),
        spaceAfter=4
    )
    
    section_head_style = ParagraphStyle(
        'SectionHead',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=18,
        leading=22,
        textColor=colors.HexColor("#1B1A17"),
        spaceAfter=8
    )
    
    body_style = ParagraphStyle(
        'SectionBody',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=10,
        leading=14,
        textColor=colors.HexColor("#1B1A17"),
        spaceAfter=12
    )

    story = []

    # ---------------- COVER PAGE ----------------
    story.append(Spacer(1, 120))
    story.append(Paragraph("TASK SCHEDULING OPTIMIZATION", title_style))
    story.append(Paragraph("Job Sequencing with Deadlines & Mathematical Verification", subtitle_style))
    
    meta_data = [
        [Paragraph("<b>Architecture:</b>", body_style), Paragraph("C99 Native Sockets HTTP Server (Port 9090) + Vanilla HTML5/CSS3/ES Modules", body_style)],
        [Paragraph("<b>Typography:</b>", body_style), Paragraph("Permanent Marker (Headings & Accents), JetBrains Mono (Data & Slots), Instrument Sans", body_style)],
        [Paragraph("<b>Algorithms:</b>", body_style), Paragraph("Greedy (O(n²)) and Exhaustive Brute-Force (O(2ⁿ)) with VERIFIED OPTIMAL validation", body_style)],
        [Paragraph("<b>Project Team:</b>", body_style), Paragraph("Sarathi R, Sohali Priyanka, Selvasiva S, Mukesh", body_style)],
        [Paragraph("<b>Local Server:</b>", body_style), Paragraph("http://localhost:9090", body_style)],
    ]
    meta_table = Table(meta_data, colWidths=[120, 390])
    meta_table.setStyle(TableStyle([
        ('VALIGN', (0,0), (-1,-1), 'TOP'),
        ('BOTTOMPADDING', (0,0), (-1,-1), 6),
        ('TOPPADDING', (0,0), (-1,-1), 6),
        ('LINEBELOW', (0,0), (-1,-1), 0.5, colors.HexColor("#E5E0D5")),
    ]))
    story.append(meta_table)
    story.append(PageBreak())

    # ---------------- SECTIONS ----------------
    sections = [
        {
            "tag": "SECTION 01",
            "title": "Jobs Inventory & Input Management",
            "desc": "Interactive job table with real-time numeric constraint validation (deadline ≥ 1, profit > 0). Supports one-click loading of benchmark files (data/jobs.txt), manual additions/deletions, or instant generation of random job sets.",
            "img": os.path.join(SCREENSHOTS_DIR, "01_jobs.png")
        },
        {
            "tag": "SECTION 02",
            "title": "Greedy Algorithm & Interactive Step Player",
            "desc": "Sorts jobs by profit in descending order and assigns each to the latest available slot. Features a full step-through playback controller (Prev, Play/Pause, Next, scrubber), a large animated running profit counter, and plain-language placement explanations.",
            "img": os.path.join(SCREENSHOTS_DIR, "02_greedy.png")
        },
        {
            "tag": "SECTION 03",
            "title": "Timetable Board Grid",
            "desc": "Visual classroom timetable layout displaying discrete time slots (T1, T2, T3, ...) with job tiles, hatched patterns for idle slots, and a struck-through list of unscheduled jobs with explicit rejection reasons.",
            "img": os.path.join(SCREENSHOTS_DIR, "03_timetable.png")
        },
        {
            "tag": "SECTION 04",
            "title": "Algorithmic Verification & Optimal Stamp",
            "desc": "Exhaustive subset exploration evaluating all 2ⁿ combinations to mathematically prove solution optimality. Displays a double-bordered forest green 'VERIFIED OPTIMAL' teacher's stamp and side-by-side metric comparison cards.",
            "img": os.path.join(SCREENSHOTS_DIR, "04_verification.png")
        },
        {
            "tag": "SECTION 05",
            "title": "Performance Complexity Analysis",
            "desc": "Logarithmic inline SVG scaling curve contrasting Greedy polynomial execution O(n²) against Brute-Force exponential growth O(2ⁿ) from n = 5 up to n = 18, accompanied by an empirical speedup table.",
            "img": os.path.join(SCREENSHOTS_DIR, "05_performance.png")
        }
    ]

    # Available printable width and height
    avail_width = 8.5 * 72 - 80  # 532 pt
    max_img_height = 430         # 430 pt

    for sec in sections:
        story.append(Paragraph(sec["tag"], section_tag_style))
        story.append(Paragraph(sec["title"], section_head_style))
        story.append(Paragraph(sec["desc"], body_style))
        story.append(Spacer(1, 6))

        # Process image and fit proportionally
        if os.path.exists(sec["img"]):
            with Image.open(sec["img"]) as im:
                orig_w, orig_h = im.size
            
            aspect = orig_h / orig_w
            target_w = avail_width
            target_h = target_w * aspect
            
            if target_h > max_img_height:
                target_h = max_img_height
                target_w = target_h / aspect
                
            rl_img = RLImage(sec["img"], width=target_w, height=target_h)
            story.append(rl_img)
        
        story.append(PageBreak())

    # Remove trailing page break if any
    story.pop()
    
    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"Generated PDF: {OUTPUT_PDF}")

    # Copy backup
    with open(OUTPUT_PDF, 'rb') as src, open(BACKUP_PDF, 'wb') as dst:
        dst.write(src.read())
    print(f"Backup copy saved: {BACKUP_PDF}")

if __name__ == '__main__':
    build_pdf()
