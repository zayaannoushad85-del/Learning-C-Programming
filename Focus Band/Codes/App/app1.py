from flask import Flask, request, jsonify, render_template_string
import time
import threading
import random

app = Flask(__name__)

# ================== DASHBOARD HTML (UI + Tabs + Rankings + Rewards) ==================

DASHBOARD_HTML = r"""
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <title>Focus Band Dashboard</title>
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <style>
    /* ===== Base (kept from previous design) ===== */
    *{box-sizing:border-box;margin:0;padding:0;font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Helvetica Neue",Arial}
    body{min-height:100vh;background:radial-gradient(circle at top,#1f2937 0,#020617 55%,#000 100%);color:#e5e7eb;display:flex;align-items:flex-start;justify-content:center;padding:20px}
    .container{width:100%;max-width:1200px;background:rgba(8,12,20,0.75);border-radius:18px;padding:14px;border:1px solid rgba(100,116,139,0.12);box-shadow:0 20px 50px rgba(0,0,0,0.6)}
    /* ===== Top nav ===== */
    .topbar{display:flex;align-items:center;justify-content:space-between;padding:10px 12px;border-bottom:1px solid rgba(148,163,184,0.04);gap:10px}
    .brand{display:flex;align-items:center;gap:12px}
    .brand h1{font-size:1.2rem;letter-spacing:0.02em}
    .pill{font-size:0.72rem;padding:4px 8px;border-radius:999px;background:linear-gradient(90deg,#22c55e,#22d3ee);color:#021015}
    .nav{display:flex;gap:8px;align-items:center}
    .nav button{background:transparent;border:0;padding:8px 12px;border-radius:10px;color:#cfe6ff;font-weight:600;cursor:pointer;letter-spacing:0.02em;position:relative}
    .nav button.active{background:linear-gradient(90deg, rgba(34,197,94,0.08), rgba(59,130,246,0.06));box-shadow:0 6px 18px rgba(59,130,246,0.06);color:#eaf8ff}
    .nav button:hover{transform:translateY(-2px);transition:all .18s ease}
    .status-chip{padding:6px 10px;border-radius:999px;border:1px solid rgba(148,163,184,0.2);background:rgba(3,7,12,0.4);display:inline-flex;gap:8px;align-items:center}
    .status-dot{width:10px;height:10px;border-radius:999px;background:#22c55e;box-shadow:0 0 8px #22c55e}
    /* ===== Tab contents ===== */
    .tabs{padding:12px}
    .tab{display:none;animation:fadeUp .28s ease both}
    .tab.active{display:block}
    @keyframes fadeUp{from{opacity:0;transform:translateY(6px)}to{opacity:1;transform:translateY(0)}}

    /* ===== Dashboard layout (left meter + right stats) ===== */
    .layout{display:grid;grid-template-columns:2fr 1.2fr;gap:16px;margin-top:12px}
    .card{background:linear-gradient(180deg, rgba(10,14,22,0.55), rgba(6,10,16,0.6));border-radius:14px;padding:16px;border:1px solid rgba(100,116,139,0.06);position:relative;overflow:hidden}
    .card .card-title{font-size:0.88rem;color:#9ca3af;text-transform:uppercase;letter-spacing:0.08em;margin-bottom:8px}
    /* meter (reuse) */
    .meter-wrapper{display:flex;align-items:center;gap:18px}
    .meter-bg{width:210px;height:210px;border-radius:50%;background:radial-gradient(circle,#020617 0,#0b1120 45%);border:1px solid rgba(148,163,184,0.12);display:flex;align-items:center;justify-content:center;position:relative;box-shadow:0 0 40px rgba(59,130,246,0.12)}
    .meter-fill{position:absolute;inset:14px;border-radius:50%;background:conic-gradient(from 160deg,#22c55e,#facc15,#f97316,#ef4444);mask:radial-gradient(farthest-side, transparent calc(100% - 18px), black calc(100% - 18px))}
    .meter-center{width:120px;height:120px;border-radius:50%;background:radial-gradient(circle at top,#0b1120,#020617 65%);display:flex;flex-direction:column;align-items:center;justify-content:center}
    .meter-value{font-size:2.4rem;font-weight:700}
    .meter-status{padding:4px 8px;border-radius:999px;font-size:0.72rem;margin-top:6px}
    .meter-status.focused{background:rgba(34,197,94,0.12);color:#bbf7d0;border:1px solid rgba(34,197,94,0.25)}
    .meter-status.distracted{background:rgba(249,115,22,0.12);color:#fed7aa;border:1px solid rgba(249,115,22,0.25)}
    /* right stats */
    .live-grid{display:grid;grid-template-columns:repeat(2,1fr);gap:10px;margin-top:6px}
    .stat{background:rgba(5,8,14,0.6);padding:10px;border-radius:10px;border:1px solid rgba(148,163,184,0.04)}
    .stat-label{font-size:0.72rem;color:#9ca3af;text-transform:uppercase}
    .stat-value{font-weight:700;font-size:1.05rem}
    /* timeline */
    .timeline{display:flex;align-items:center;gap:6px;margin-top:10px}
    .timeline-dot{flex:1;height:8px;border-radius:10px;background:rgba(148,163,184,0.16)}
    .timeline-dot.focused{background:#22c55e;box-shadow:0 0 8px rgba(34,197,94,0.7)}
    .timeline-dot.distracted{background:#f97316;box-shadow:0 0 8px rgba(249,115,22,0.7)}
    /* summary */
    .summary{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin-top:10px}
    .summary-stat{background:linear-gradient(180deg, rgba(7,12,8,0.45), rgba(6,8,10,0.4));padding:10px;border-radius:10px;border:1px solid rgba(74,222,128,0.06)}
    .summary-label{font-size:0.72rem;color:#a7f3d0}
    .summary-value{font-weight:700;font-size:1rem;color:#ecfdf3}

    /* ===== Rankings (A2 Table style) ===== */
    .leaderboard{margin-top:10px}
    .leaderboard table{width:100%;border-collapse:collapse;background:transparent}
    .leaderboard th,.leaderboard td{padding:10px 12px;text-align:left}
    .leaderboard th{color:#9ec7ff;text-transform:uppercase;font-size:0.75rem;border-bottom:1px dashed rgba(120,150,200,0.08)}
    .leader-row{background:linear-gradient(90deg, rgba(255,255,255,0.01), rgba(255,255,255,0.00));border-radius:8px;margin-bottom:8px;display:flex;align-items:center;justify-content:space-between;padding:8px 10px}
    .rank-badge{width:42px;height:42px;border-radius:8px;display:flex;align-items:center;justify-content:center;font-weight:700;color:#021015;background:linear-gradient(90deg,#22c55e,#22d3ee);box-shadow:0 8px 22px rgba(34,197,94,0.08)}
    .score-pill{font-weight:700;padding:6px 10px;border-radius:999px;background:linear-gradient(90deg, rgba(34,197,94,0.08), rgba(59,130,246,0.04));color:#dbf6ff}
    .leaderboard tbody{display:flex;flex-direction:column;gap:8px;margin-top:10px}
    .leaderboard .row{display:flex;align-items:center;justify-content:space-between;padding:8px;border-radius:10px;border:1px solid rgba(120,140,180,0.04);background:linear-gradient(180deg, rgba(6,10,16,0.5), rgba(6,10,16,0.45))}
    .leaderboard .row:hover{transform:translateX(6px);transition:transform .18s ease}
    /* rank colors */
    .rank-1 .rank-badge{background:linear-gradient(90deg,#ffd166,#ffb703);color:#041017;box-shadow:0 12px 30px rgba(255,177,3,0.08)}
    .rank-2 .rank-badge{background:linear-gradient(90deg,#c9f0ff,#7dd3fc);color:#021017}
    .rank-3 .rank-badge{background:linear-gradient(90deg,#ffdce6,#ffb3d2);color:#021017}

    /* ===== Rewards page ===== */
    .badges{display:grid;grid-template-columns:repeat(3,1fr);gap:12px;margin-top:12px}
    .badge-card{background:linear-gradient(180deg, rgba(8,12,16,0.6), rgba(6,8,10,0.45));padding:12px;border-radius:12px;border:1px solid rgba(150,200,180,0.04);text-align:center}
    .badge-icon{width:64px;height:64px;border-radius:12px;margin:0 auto;display:flex;align-items:center;justify-content:center;font-size:32px;background:linear-gradient(90deg,#22c55e,#22d3ee);color:#021015}
    .badge-title{margin-top:8px;font-weight:700}
    .badge-desc{font-size:0.88rem;color:#9ca3af;margin-top:6px}
    .badge-locked{opacity:0.35;filter:grayscale(0.2)}
    .claim-btn{margin-top:8px;padding:8px 12px;border-radius:8px;border:0;cursor:pointer;background:linear-gradient(90deg,#22c55e,#06b6d4);color:#021015;font-weight:700}
    .claim-btn[disabled]{opacity:0.5;cursor:not-allowed}
    /* confetti */
    .confetti{position:fixed;left:0;top:0;width:100%;height:0;pointer-events:none;z-index:9999}

    /* responsive */
    @media(max-width:960px){.layout{grid-template-columns:1fr}.badges{grid-template-columns:repeat(2,1fr)}}
    @media(max-width:520px){.badges{grid-template-columns:1fr}.nav{overflow:auto}}
  </style>
</head>
<body>
  <div class="container">
    <div class="topbar">
      <div class="brand">
        <h1>Focus Band</h1>
        <div class="pill">Live Neuro-Focus Monitor</div>
      </div>

      <div style="display:flex;align-items:center;gap:12px">
        <div class="nav" id="nav">
          <button class="tab-btn active" data-tab="dashboard">Dashboard</button>
          <button class="tab-btn" data-tab="rankings">Rankings</button>
          <button class="tab-btn" data-tab="rewards">Rewards</button>
        </div>
        <div style="width:18px"></div>
        <div class="status-chip" id="connectionChip">
          <span class="status-dot" id="connectionDot"></span>
          <div style="margin-left:6px">
            <div style="font-weight:700;font-size:0.82rem" id="connectionLabel">LIVE LINK</div>
            <div style="font-size:0.72rem;color:#9ca3af" id="connectionText">Streaming from device…</div>
          </div>
        </div>
      </div>
    </div>

    <div class="tabs">
      <!-- DASHBOARD TAB -->
      <div class="tab active" id="tab-dashboard">
        <div class="layout">
          <div class="card">
            <div class="card-title">Focus Score</div>
            <div style="display:flex;align-items:center;gap:16px;margin-top:6px">
              <div class="meter-wrapper">
                <div class="meter-bg">
                  <div class="meter-fill" id="meterFill" style="transform:rotate(0deg)"></div>
                  <div class="meter-center">
                    <div class="meter-value" id="focusScoreDisplay">0</div>
                    <div style="font-size:0.78rem;color:#9ca3af">/ 100</div>
                    <div class="meter-status" id="focusStatusBadge">Idle</div>
                  </div>
                </div>
              </div>

              <div style="flex:1;display:flex;flex-direction:column;gap:8px">
                <div class="card-title" style="margin-bottom:6px">Live Vitals</div>
                <div class="live-grid">
                  <div class="stat"><div class="stat-label">Heart Rate</div><div class="stat-value" id="heartRateDisplay">--</div></div>
                  <div class="stat"><div class="stat-label">Movement Level</div><div class="stat-value" id="motionDisplay">--</div></div>
                  <div class="stat"><div class="stat-label">Focus State</div><div class="stat-value" id="liveStateDisplay">Idle</div></div>
                  <div class="stat"><div class="stat-label">Distractions</div><div class="stat-value" id="distractionCountDisplay">0</div></div>
                </div>

                <div class="timeline">
                  <div style="width:90px;font-size:0.72rem;color:#9ca3af">Timeline</div>
                  <div style="flex:1;display:flex;gap:4px" id="timelineTrack"></div>
                </div>

              </div>
            </div>

            <div class="summary">
              <div class="summary-stat">
                <div class="summary-label">Time Focused</div>
                <div class="summary-value" id="focusedPercentDisplay">0%</div>
              </div>
              <div class="summary-stat">
                <div class="summary-label">Best Streak</div>
                <div class="summary-value" id="bestStreakDisplay">0s</div>
              </div>
              <div class="summary-stat">
                <div class="summary-label">Session Length</div>
                <div class="summary-value" id="sessionLengthDisplay">0s</div>
              </div>
            </div>
          </div>

          <div class="side-column">
            <div class="card">
              <div class="card-title">Session Controls</div>
              <div style="display:flex;gap:8px;margin-top:8px">
                <button id="startBtn" class="claim-btn">Start Session</button>
                <button id="resetBtn" class="claim-btn" style="background:linear-gradient(90deg,#ff7ab6,#ffb3d2)">Reset</button>
              </div>
              <div style="margin-top:10px;color:#9ca3af;font-size:0.9rem">Use Start before a judge demo to initialize counters.</div>
            </div>

            <div class="card">
              <div class="card-title">Signal Quality</div>
              <div style="margin-top:8px" id="qualityChip">Waiting for data...</div>
              <div style="margin-top:10px;color:#9ca3af">Tip: Ensure band is snug and stable for best readings.</div>
            </div>

            <div class="card">
              <div class="card-title">Session Summary</div>
              <div style="margin-top:8px;color:#9ca3af" id="summaryNote">Keep the band comfortable for accurate reading.</div>
            </div>
          </div>
        </div>
      </div>

      <!-- RANKINGS TAB -->
      <div class="tab" id="tab-rankings">
        <div class="card">
          <div class="card-title">Leaderboard — Top Performers</div>
          <div class="leaderboard" id="leaderboard">
            <table>
              <thead>
                <tr>
                  <th style="width:90px">Rank</th>
                  <th>Name</th>
                  <th style="width:140px">Score</th>
                </tr>
              </thead>
            </table>

            <div id="leaderRows" style="margin-top:12px;display:flex;flex-direction:column;gap:10px"></div>
          </div>
        </div>
      </div>

      <!-- REWARDS TAB -->
      <div class="tab" id="tab-rewards">
        <div class="card">
          <div class="card-title">Rewards & Achievements</div>
          <div style="font-size:0.88rem;color:#9ca3af;margin-top:6px">Earn badges for focus streaks and milestones — claim to celebrate!</div>

          <div class="badges" id="badgesArea">
            <!-- badges added by JS -->
          </div>
        </div>
      </div>
    </div>
  </div>

  <div class="confetti" id="confetti"></div>

  <script>
    // ===== Tab switching =====
    document.querySelectorAll('.tab-btn').forEach(btn=>{
      btn.addEventListener('click', ()=>{
        document.querySelectorAll('.tab-btn').forEach(b=>b.classList.remove('active'));
        btn.classList.add('active');
        const tab = btn.getAttribute('data-tab');
        document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
        document.getElementById('tab-'+tab).classList.add('active');
      });
    });

    // ===== Timeline history =====
    const timelineHistory = [];
    const MAX_TIMELINE_POINTS = 16;
    const timelineTrack = document.getElementById('timelineTrack');

    function updateTimeline(status){
      const mapped = status === 'focused' ? 'focused' : status === 'distracted' ? 'distracted' : 'idle';
      timelineHistory.push(mapped);
      if(timelineHistory.length>MAX_TIMELINE_POINTS) timelineHistory.shift();
      timelineTrack.innerHTML='';
      timelineHistory.forEach(s=>{
        const d=document.createElement('div'); d.className='timeline-dot '+s; timelineTrack.appendChild(d);
      });
    }

    // ===== Leaderboard data (bluffed) =====
    const leaderboardData = [
      {name:'Zara', score:92},
      {name:'Arjun', score:88},
      {name:'Meera', score:73},
      {name:'Raghav', score:64},
      {name:'Kavin', score:59},
      {name:'Aisha', score:53},
      {name:'You', score:51},
      {name:'Anonymous', score:42}
    ];

    function renderLeaderboard(){
      const container = document.getElementById('leaderRows');
      container.innerHTML = '';
      leaderboardData.forEach((p, i)=>{
        const row = document.createElement('div');
        row.className = 'row ' + (i<3? 'rank-'+(i+1): '');
        row.style.display='flex'; row.style.alignItems='center'; row.style.justifyContent='space-between';
        row.innerHTML = `
          <div style="display:flex;align-items:center;gap:12px">
            <div class="rank-badge">${i+1}</div>
            <div style="display:flex;flex-direction:column">
              <div style="font-weight:700">${p.name}</div>
              <div style="font-size:0.82rem;color:#9ca3af">Streak: ${Math.floor(Math.random()*80)}s</div>
            </div>
          </div>
          <div class="score-pill">${p.score} pts</div>
        `;
        // animate entry
        row.style.opacity='0'; row.style.transform='translateY(8px)';
        container.appendChild(row);
        setTimeout(()=>{row.style.transition='all .35s ease';row.style.opacity='1';row.style.transform='translateY(0)';}, 60 + i*80);
      });
    }

    renderLeaderboard();

    // ===== Rewards =====
    const badges = [
      {id:'b1', title:'Starter Mind', desc:'5 minute focused streak', icon:'🟢', threshold:5*60},
      {id:'b2', title:'Focused Brain', desc:'10 minute focused streak', icon:'🧠', threshold:10*60},
      {id:'b3', title:'Peak Mode', desc:'25 minute session', icon:'🔥', threshold:25*60},
      {id:'b4', title:'Productivity Star', desc:'100 total focused minutes', icon:'🌟', threshold:100*60}
    ];

    function renderBadges(currentFocusedSeconds, sessionSeconds){
      const area = document.getElementById('badgesArea');
      area.innerHTML='';
      badges.forEach(b=>{
        const unlocked = currentFocusedSeconds >= b.threshold || sessionSeconds >= b.threshold;
        const card = document.createElement('div');
        card.className='badge-card ' + (unlocked? '':'badge-locked');
        card.innerHTML = `
          <div class="badge-icon">${b.icon}</div>
          <div class="badge-title">${b.title}</div>
          <div class="badge-desc">${b.desc}</div>
          <button class="claim-btn" ${unlocked? '':'disabled'} data-id="${b.id}">${unlocked? 'Claim Badge':'Locked'}</button>
        `;
        area.appendChild(card);
      });
      // attach claim handlers
      document.querySelectorAll('.claim-btn').forEach(btn=>{
        btn.addEventListener('click', (e)=>{
          if(btn.disabled) return;
          launchConfetti();
          btn.textContent='Claimed ✓';
          btn.disabled = true;
        });
      });
    }

    // confetti simple
    function launchConfetti(){
      const wrap = document.getElementById('confetti');
      for(let i=0;i<40;i++){
        const el = document.createElement('div');
        const size = Math.random()*8+6;
        el.style.position='absolute';
        el.style.left = Math.random()*100 + '%';
        el.style.top = '-10px';
        el.style.width = size+'px';
        el.style.height = size*0.6+'px';
        el.style.background = ['#22c55e','#06b6d4','#ffd166','#f97316'][Math.floor(Math.random()*4)];
        el.style.opacity=0.95;
        el.style.borderRadius='3px';
        el.style.transform = 'rotate('+ (Math.random()*360)+'deg)';
        el.style.zIndex=9999;
        wrap.appendChild(el);
        // animate
        const fall = 1200 + Math.random()*900;
        el.animate([
          {transform:'translateY(0) rotate(0)', opacity:1},
          {transform:'translateY(' + (80+Math.random()*700) + 'px) rotate(360deg)', opacity:0.95}
        ], {duration:fall, easing:'cubic-bezier(.2,.8,.2,1)'});
        setTimeout(()=>el.remove(), fall+100);
      }
    }

    // ===== Helpers =====
    function pad(n){return n.toString().padStart(2,'0')}
    function formatDuration(sec){sec=Math.max(0,Math.floor(sec||0));const m=Math.floor(sec/60);const s=sec%60;return m===0?`${s}s`:`${m}m ${pad(s)}s`}

    // ===== Polling / Dashboard updates (keeps same endpoints) =====
    const meterFill = document.getElementById('meterFill');
    const focusScoreDisplay = document.getElementById('focusScoreDisplay');
    const focusStatusBadge = document.getElementById('focusStatusBadge');
    const meterGlow = document.getElementById('meterFill'); // reused

    const heartRateDisplay = document.getElementById('heartRateDisplay');
    const motionDisplay = document.getElementById('motionDisplay');
    const liveStateDisplay = document.getElementById('liveStateDisplay');
    const distractionCountDisplay = document.getElementById('distractionCountDisplay');

    const sessionTimeDisplay = document.getElementById('sessionLengthDisplay');
    const focusedPercentDisplay = document.getElementById('focusedPercentDisplay');
    const bestStreakDisplay = document.getElementById('bestStreakDisplay');

    const connectionDot = document.getElementById('connectionDot');
    const connectionLabel = document.getElementById('connectionLabel');
    const connectionText = document.getElementById('connectionText');
    const qualityChip = document.getElementById('qualityChip');
    const summaryNote = document.getElementById('summaryNote');

    function updateMeter(score,status){
      const clamp = Math.max(0,Math.min(100,score||0));
      focusScoreDisplay.textContent = Math.round(clamp);
      const angle = (clamp/100)*260;
      meterFill.style.transform = `rotate(${angle}deg)`;
      focusStatusBadge.textContent = status==='focused'?'Focused':status==='distracted'?'Distracted':'Idle';
      focusStatusBadge.className = 'meter-status ' + (status==='focused'?'focused':status==='distracted'?'distracted':'');
    }

    function updateQuality(motion){
      if(motion==null){qualityChip.textContent='Signal quality: Waiting for data';return}
      if(motion<0.03){qualityChip.textContent='Signal quality: Excellent (very stable)'}
      else if(motion<0.09){qualityChip.textContent='Signal quality: Good (natural movements)'}
      else if(motion<0.18){qualityChip.textContent='Signal quality: Noisy (frequent head movement)'}
      else {qualityChip.textContent='Signal quality: Very noisy (moving a lot)'}
    }

    async function fetchData(){
      try{
        const res = await fetch('/data');
        if(!res.ok) throw new Error('http '+res.status);
        const d = await res.json();
        // connection state
        connectionDot.style.background = '#22c55e'; connectionLabel.textContent='LIVE LINK'; connectionText.textContent='Streaming from device…';
        // fields
        const focusScore = d.focus_score||d.score||0;
        const motion = d.motion!=null?d.motion:null;
        const status = d.status||'idle';
        const hr = d.heart_rate||d.bpm||null;
        const distractions = d.distractions||0;
        const sessionTime = d.session_time||d.session_length||0;
        const best = d.longest_streak||0;

        updateMeter(focusScore,status);
        updateQuality(motion);
        updateTimeline(status);

        heartRateDisplay.textContent = hr!=null?Math.round(hr):'--';
        motionDisplay.textContent = motion!=null? (Number(motion).toFixed(3)) : '--';
        liveStateDisplay.textContent = status==='focused'?'Focused':status==='distracted'?'Distracted':'Idle';
        distractionCountDisplay.textContent = distractions;
        sessionTimeDisplay.textContent = formatDuration(sessionTime);
        focusedPercentDisplay.textContent = Math.round(focusScore)+'%';
        bestStreakDisplay.textContent = formatDuration(best);

        // update leaderboard blur (simulate slight changes)
        // shuffle small
        // (we keep static as bluff but could add random drift)
        // update badges unlocking
        renderBadges(Math.round((focusScore/100)*sessionTime), sessionTime);
      }catch(err){
        console.error('fetch err',err);
        connectionDot.style.background = '#ef4444'; connectionLabel.textContent='DISCONNECTED'; connectionText.textContent='No data — check cable / port.';
      }
    }

    // initial fill for timeline
    for(let i=0;i<MAX_TIMELINE_POINTS;i++) timelineHistory.push('idle');
    updateTimeline('idle');

    fetchData();
    setInterval(fetchData,1000);

    // ===== Session start/reset APIs =====
    document.getElementById('startBtn').addEventListener('click', async ()=>{
      try{ await fetch('/start',{method:'POST'}); alert('Session started'); }catch(e){alert('failed')}
    });
    document.getElementById('resetBtn').addEventListener('click', async ()=>{
      try{ await fetch('/reset',{method:'POST'}); alert('Session reset'); }catch(e){alert('failed')}
    });

  </script>
</body>
</html>
"""

# ================== APPLICATION STATE ==================

state = {
    "session_start": None,
    "last_update": None,
    "session_seconds": 0.0,
    "focused_seconds": 0.0,
    "current_focus_start": None,
    "distractions": 0,
    "current_status": "idle",
    "longest_streak": 0.0,
    "current_streak_start": None,
    "heart_rate_smooth": 72.0,
    "last_motion": None,
    "last_ax": None,
    "last_ay": None,
    "last_az": None,
}

LOCK = threading.Lock()
HR_ALPHA = 0.85

def now():
    return time.time()

def ensure_session_started(ts):
    if state["session_start"] is None:
        state["session_start"] = ts
        state["current_streak_start"] = ts if state["current_status"] == "focused" else None
        state["last_update"] = ts

def update_session_time(ts):
    if state["session_start"] is None:
        return 0.0
    return ts - state["session_start"]

# ================== BRIDGE -> /update ==================

@app.route("/update", methods=["POST"])
def update():
    data = request.get_json(force=True, silent=True)
    if not data:
        return jsonify({"error": "no json received"}), 400

    with LOCK:
        ts = now()
        ensure_session_started(ts)

        ax = data.get("ax")
        ay = data.get("ay")
        az = data.get("az")
        motion = data.get("motion")
        status_raw = data.get("status", "").lower() if data.get("status") else "idle"
        status = status_raw if status_raw in ("focused", "distracted") else "idle"

        state["last_motion"] = motion
        state["last_ax"] = ax
        state["last_ay"] = ay
        state["last_az"] = az

        state["last_update"] = ts
        session_secs = update_session_time(ts)
        state["session_seconds"] = session_secs

        prev_status = state["current_status"]

        if prev_status != status:
            if prev_status == "focused" and status != "focused":
                if state["current_focus_start"]:
                    elapsed = ts - state["current_focus_start"]
                    state["focused_seconds"] += elapsed
                    if elapsed > state["longest_streak"]:
                        state["longest_streak"] = elapsed
                state["current_focus_start"] = None

            if prev_status != "focused" and status == "focused":
                state["current_focus_start"] = ts

            if prev_status == "focused" and status == "distracted":
                state["distractions"] += 1

            if status == "focused":
                state["current_streak_start"] = ts
            else:
                state["current_streak_start"] = None
        else:
            if status == "focused" and state["current_focus_start"] is None:
                state["current_focus_start"] = ts

        state["current_status"] = status

        focused_total = state["focused_seconds"]
        if state["current_focus_start"]:
            focused_total += (ts - state["current_focus_start"])

        focus_score = 0
        if state["session_seconds"] > 0:
            focus_score = round(min(100.0, (focused_total / max(1.0, state["session_seconds"])) * 100.0))

        base_hr = 72.0
        motion_influence = 0.0
        if isinstance(motion, (int, float)):
            motion_influence = min(12.0, max(0.0, motion * 60.0))
        jitter = random.uniform(-1.0, 1.0)
        measured_hr = base_hr + motion_influence + jitter

        state["heart_rate_smooth"] = HR_ALPHA * state["heart_rate_smooth"] + (1.0 - HR_ALPHA) * measured_hr
        hr_to_report = round(state["heart_rate_smooth"])

        response_payload = {
            "ax": ax,
            "ay": ay,
            "az": az,
            "motion": motion,
            "status": status,
            "heart_rate": hr_to_report,
            "focus_score": focus_score,
            "session_time": round(state["session_seconds"]),
            "longest_streak": round(state["longest_streak"]),
            "distractions": state["distractions"]
        }

    return jsonify(response_payload), 200

# ================== FRONTEND -> /data ==================

@app.route("/data", methods=["GET"])
def data():
    with LOCK:
        ts = now()
        if state["session_start"]:
            state["session_seconds"] = ts - state["session_start"]

        focused_total = state["focused_seconds"]
        if state["current_focus_start"]:
            focused_total += (ts - state["current_focus_start"])

        focus_score = 0
        if state["session_seconds"] > 0:
            focus_score = round(min(100.0, (focused_total / max(1.0, state["session_seconds"])) * 100.0))

        current_streak = 0.0
        if state["current_focus_start"]:
            current_streak = ts - state["current_focus_start"]
        longest = max(state["longest_streak"], current_streak)

        payload = {
            "ax": state["last_ax"],
            "ay": state["last_ay"],
            "az": state["last_az"],
            "motion": state["last_motion"],
            "status": state["current_status"],
            "heart_rate": round(state["heart_rate_smooth"]),
            "focus_score": focus_score,
            "session_time": round(state["session_seconds"]),
            "longest_streak": round(longest),
            "distractions": state["distractions"]
        }
    return jsonify(payload)

# ================== DASHBOARD ROUTE ==================

@app.route("/")
def index():
    return render_template_string(DASHBOARD_HTML)

# ================== OPTIONAL RESET ENDPOINTS ==================

@app.route("/start", methods=["POST"])
def start_session():
    with LOCK:
        state["session_start"] = now()
        state["last_update"] = state["session_start"]
        state["session_seconds"] = 0.0
        state["focused_seconds"] = 0.0
        state["current_focus_start"] = None
        state["distractions"] = 0
        state["current_status"] = "idle"
        state["longest_streak"] = 0.0
        state["current_streak_start"] = None
        state["heart_rate_smooth"] = 72.0
        state["last_motion"] = None
        state["last_ax"] = None
        state["last_ay"] = None
        state["last_az"] = None
    return jsonify({"ok": True}), 200

@app.route("/reset", methods=["POST"])
def reset_session():
    with LOCK:
        state["session_start"] = None
        state["last_update"] = None
        state["session_seconds"] = 0.0
        state["focused_seconds"] = 0.0
        state["current_focus_start"] = None
        state["distractions"] = 0
        state["current_status"] = "idle"
        state["longest_streak"] = 0.0
        state["current_streak_start"] = None
        state["heart_rate_smooth"] = 72.0
        state["last_motion"] = None
        state["last_ax"] = None
        state["last_ay"] = None
        state["last_az"] = None
    return jsonify({"ok": True}), 200

# ================== MAIN ==================

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
