/**
 * ==============================================================================
 * SmartRoute CGC — Intelligent Campus Navigation Using Graph Algorithms
 * Real-World C++ DSA Full-Stack Application for CGC Landran, Mohali
 * Google Earth 3D Perspective & Cinematic Campus Engine
 * ==============================================================================
 */

// Application Global State
const State = {
  nodes: [],
  edges: [],
  nodeMap: new Map(),
  activeRoute: null,
  activeAlgorithm: 'astar',
  activeMode: 'walking',
  simNavInterval: null,
  simNavStep: 0,
  
  // 3D Google Earth Camera State
  is3D: true,
  pitch: 46,           // Camera tilt angle in degrees (0 = 2D plan, 46 = 3D perspective)
  bearing: -12,        // Azimuth orientation angle in degrees
  zoomLevel: 1.05,
  panX: 0,
  panY: 20,
  isOrbiting: false,
  orbitRafId: null,

  // Drag interaction state
  isDragging: false,
  isRightDragging: false,
  dragStartX: 0,
  dragStartY: 0,
  startBearing: 0,
  startPitch: 0,

  // Viewport Toggles
  showBuildings: true,
  showLabels: true,
  showTraffic: true,
  showGreenery: true,
  inspectedNodeId: null
};

// SVG Viewport Calibration
const SVG_CONFIG = {
  viewWidth: 1000,
  viewHeight: 750,
  padX: 90,
  padY: 65,
  minX: 1.0,
  maxX: 9.2,
  minY: 0.8,
  maxY: 10.0
};

/**
 * Transforms Cartesian graph coordinates to SVG pixel space.
 * Inverts Y so that higher Y values (North) appear near the top.
 */
function toSvgCoords(x, y) {
  const normX = (x - SVG_CONFIG.minX) / (SVG_CONFIG.maxX - SVG_CONFIG.minX);
  const normY = (y - SVG_CONFIG.minY) / (SVG_CONFIG.maxY - SVG_CONFIG.minY);

  const usableWidth = SVG_CONFIG.viewWidth - (2 * SVG_CONFIG.padX);
  const usableHeight = SVG_CONFIG.viewHeight - (2 * SVG_CONFIG.padY);

  const svgX = SVG_CONFIG.padX + (normX * usableWidth);
  const svgY = SVG_CONFIG.viewHeight - SVG_CONFIG.padY - (normY * usableHeight);

  return { x: Math.round(svgX), y: Math.round(svgY) };
}

// Building footprint geometry definitions with 3D architectural height (dz)
const BUILDING_DIMENSIONS = {
  0: { w: 56, h: 26, dz: 14, label: "MAIN GATE 1", type: "gate" },
  1: { w: 68, h: 42, dz: 10, label: "VISITOR PARKING", type: "parking" },
  2: { w: 76, h: 48, dz: 28, label: "ADMIN BLOCK", type: "admin" },
  3: { w: 78, h: 54, dz: 26, label: "CEC BLOCK 1", type: "academic" },
  4: { w: 84, h: 58, dz: 32, label: "CSE & IT (BLOCK 2)", type: "academic" },
  5: { w: 82, h: 52, dz: 34, label: "CENTRAL LIBRARY", type: "library" },
  6: { w: 100, h: 64, dz: 0,  label: "CENTRAL LAWN", type: "lawn" },
  7: { w: 88, h: 56, dz: 30, label: "AUDITORIUM", type: "auditorium" },
  8: { w: 74, h: 46, dz: 20, label: "CANTEEN & FOOD", type: "food" },
  9: { w: 72, h: 48, dz: 22, label: "APPLIED SCI (B-3)", type: "academic" },
  10: { w: 76, h: 44, dz: 22, label: "WORKSHOP LABS", type: "workshop" },
  11: { w: 80, h: 50, dz: 26, label: "ADV COMPUTING (B-4)", type: "academic" },
  12: { w: 82, h: 50, dz: 26, label: "ECE & ELEC (B-5)", type: "academic" },
  13: { w: 86, h: 52, dz: 28, label: "CBSA (BLOCK 6)", type: "academic" },
  14: { w: 78, h: 50, dz: 26, label: "PHARMACY (CCOP)", type: "academic" },
  15: { w: 76, h: 48, dz: 24, label: "HOTEL MGMT (CCHM)", type: "academic" },
  16: { w: 92, h: 54, dz: 28, label: "BOYS HOSTELS 1 & 2", type: "hostel" },
  17: { w: 92, h: 54, dz: 28, label: "GIRLS HOSTEL", type: "hostel" },
  18: { w: 140, h: 75, dz: 12, label: "SPORTS STADIUM", type: "sports" },
  19: { w: 68, h: 44, dz: 24, label: "GYMNASIUM & TT", type: "sports" },
  20: { w: 62, h: 40, dz: 20, label: "DISPENSARY", type: "medical" },
  21: { w: 85, h: 44, dz: 10, label: "BUS BAY & FLEET", type: "parking" },
  22: { w: 68, h: 40, dz: 10, label: "2-WHEELER PARKING", type: "parking" },
  23: { w: 56, h: 26, dz: 14, label: "NORTH GATE 2", type: "gate" },
  24: { w: 78, h: 46, dz: 26, label: "BIOTECH (BLOCK 9)", type: "academic" },
  25: { w: 66, h: 46, dz: 12, label: "OPEN AIR THEATRE", type: "amenity" }
};

// =============================================================================
// UI NOTIFICATIONS & TOASTS
// =============================================================================

function showToast(message, type = 'info') {
  const shelf = document.getElementById('toast-shelf');
  if (!shelf) return;

  const toast = document.createElement('div');
  toast.className = `toast toast-${type}`;
  toast.innerHTML = `<span>${message}</span>`;
  shelf.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateY(12px)';
    setTimeout(() => toast.remove(), 350);
  }, 3200);
}

// =============================================================================
// DATA FETCHING & API INTERACTION
// =============================================================================

async function fetchCampusData() {
  try {
    const res = await fetch('/api/campus');
    if (!res.ok) throw new Error('C++ navigation engine returned ' + res.status);
    const data = await res.json();

    State.nodes = data.nodes || [];
    State.edges = data.edges || [];
    State.nodeMap.clear();
    State.nodes.forEach(n => State.nodeMap.set(n.id, n));

    populateLocationDropdowns();
    populateRoadDropdowns();
    renderSvgMap();
    fetchStatistics();

    // Sync button state for 3D Earth
    const btn3d = document.getElementById('btn-toggle-3d');
    if (btn3d) btn3d.classList.toggle('active', State.is3D);
    applySvgTransform();
  } catch (err) {
    console.error('Campus fetch error:', err);
    showToast('Connecting to C++ Navigation Server...', 'warning');
    setTimeout(fetchCampusData, 1800);
  }
}

async function fetchStatistics() {
  try {
    const res = await fetch('/api/statistics');
    if (!res.ok) return;
    const stats = await res.json();

    const nodesEl = document.getElementById('stat-nodes-count');
    const roadsEl = document.getElementById('stat-roads-count');
    const trafficEl = document.getElementById('stat-traffic-count');
    const blockedEl = document.getElementById('stat-blocked-count');

    if (nodesEl) nodesEl.textContent = stats.total_locations || State.nodes.length;
    if (roadsEl) roadsEl.textContent = stats.total_roads || State.edges.length;
    if (trafficEl) trafficEl.textContent = stats.congested_roads || 0;
    if (blockedEl) blockedEl.textContent = stats.blocked_roads || 0;
  } catch (err) {
    console.error('Stats error:', err);
  }
}

// =============================================================================
// DROPDOWN & SELECTOR POPULATION
// =============================================================================

function populateLocationDropdowns() {
  const srcSelect = document.getElementById('select-source');
  const destSelect = document.getElementById('select-dest');
  const travSelect = document.getElementById('select-traversal-start');

  if (!srcSelect || !destSelect) return;

  const prevSrc = srcSelect.value;
  const prevDest = destSelect.value;

  srcSelect.innerHTML = '<option value="" disabled selected>Select Starting Landmark</option>';
  destSelect.innerHTML = '<option value="" disabled selected>Select Destination</option>';
  if (travSelect) travSelect.innerHTML = '';

  State.nodes.forEach(n => {
    const optSrc = document.createElement('option');
    optSrc.value = n.id;
    optSrc.textContent = `[${n.id}] ${n.name}`;
    srcSelect.appendChild(optSrc);

    const optDest = document.createElement('option');
    optDest.value = n.id;
    optDest.textContent = `[${n.id}] ${n.name}`;
    destSelect.appendChild(optDest);

    if (travSelect) {
      const optTrav = document.createElement('option');
      optTrav.value = n.id;
      optTrav.textContent = `[${n.id}] ${n.name}`;
      travSelect.appendChild(optTrav);
    }
  });

  // Default selections: Main Gate 1 to CSE & IT Department (Block 2)
  if (prevSrc && State.nodeMap.has(Number(prevSrc))) {
    srcSelect.value = prevSrc;
  } else if (State.nodeMap.has(0)) {
    srcSelect.value = "0";
  }

  if (prevDest && State.nodeMap.has(Number(prevDest))) {
    destSelect.value = prevDest;
  } else if (State.nodeMap.has(4)) {
    destSelect.value = "4";
  }
}

function populateRoadDropdowns() {
  const trafficRoadSelect = document.getElementById('select-traffic-road');
  const blockRoadSelect = document.getElementById('select-block-road');

  if (trafficRoadSelect) trafficRoadSelect.innerHTML = '';
  if (blockRoadSelect) blockRoadSelect.innerHTML = '';

  State.edges.forEach(e => {
    const text = `${e.from} ⇄ ${e.to} (${e.distance_km} km)`;
    const val = `${e.srcId}-${e.destId}`;

    if (trafficRoadSelect) {
      const opt = document.createElement('option');
      opt.value = val;
      opt.textContent = text;
      trafficRoadSelect.appendChild(opt);
    }

    if (blockRoadSelect) {
      const opt = document.createElement('option');
      opt.value = val;
      opt.textContent = `${e.blocked ? '🔴 [BLOCKED] ' : '🟢 '} ${text}`;
      blockRoadSelect.appendChild(opt);
    }
  });
}

// =============================================================================
// VECTOR / SVG DIGITAL TWIN MAP RENDERING
// =============================================================================

function renderSvgMap() {
  renderBoundaryLayer();
  renderGreeneryLayer();
  renderZonesLayer();
  renderRoadsLayer();
  renderBuildingsLayer();
  renderNodesLayer();
  renderLabelsLayer();
  updateHighlightedNodes();
}

function renderBoundaryLayer() {
  const layer = document.getElementById('layer-boundary');
  if (!layer) return;
  layer.innerHTML = '';

  // Organic perimeter polygon for CGC Landran Campus (Sector 112)
  const poly = document.createElementNS('http://www.w3.org/2000/svg', 'polygon');
  poly.setAttribute('class', 'svg-campus-boundary');
  poly.setAttribute('points', '55,710 945,710 940,110 850,55 520,50 460,50 120,65 55,240 50,680');
  layer.appendChild(poly);

  // Exterior Highway (Kharar-Banur Road / NH 205A) at bottom
  const hwyOuter = document.createElementNS('http://www.w3.org/2000/svg', 'line');
  hwyOuter.setAttribute('x1', '20');
  hwyOuter.setAttribute('y1', '722');
  hwyOuter.setAttribute('x2', '980');
  hwyOuter.setAttribute('y2', '722');
  hwyOuter.setAttribute('class', 'svg-highway');
  layer.appendChild(hwyOuter);

  const hwyCenter = document.createElementNS('http://www.w3.org/2000/svg', 'line');
  hwyCenter.setAttribute('x1', '20');
  hwyCenter.setAttribute('y1', '722');
  hwyCenter.setAttribute('x2', '980');
  hwyCenter.setAttribute('y2', '722');
  hwyCenter.setAttribute('class', 'svg-highway-center');
  layer.appendChild(hwyCenter);

  // Highway Label
  const hwyText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
  hwyText.setAttribute('x', '500');
  hwyText.setAttribute('y', '742');
  hwyText.setAttribute('fill', '#94a3b8');
  hwyText.setAttribute('font-size', '10');
  hwyText.setAttribute('letter-spacing', '2');
  hwyText.setAttribute('text-anchor', 'middle');
  hwyText.setAttribute('font-family', 'JetBrains Mono, monospace');
  hwyText.textContent = '◀ KHARAR-BANUR HIGHWAY (SECTOR 112 LANDRAN) ▶';
  layer.appendChild(hwyText);
}

function renderGreeneryLayer() {
  const layer = document.getElementById('layer-greenery');
  if (!layer) return;
  layer.innerHTML = '';
  if (!State.showGreenery) return;

  // 1. Central Lawn & Rose Garden (around Node 6: x=5.0, y=4.0)
  const lawnPt = toSvgCoords(5.0, 4.0);
  const lawn = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
  lawn.setAttribute('x', lawnPt.x - 70);
  lawn.setAttribute('y', lawnPt.y - 45);
  lawn.setAttribute('width', '140');
  lawn.setAttribute('height', '90');
  lawn.setAttribute('rx', '24');
  lawn.setAttribute('class', 'svg-lawn');
  layer.appendChild(lawn);

  // 2. Sports Complex Stadium Running Track & Cricket Pitch (Node 18: x=4.5, y=8.8)
  const sportsPt = toSvgCoords(4.5, 8.8);
  const track = document.createElementNS('http://www.w3.org/2000/svg', 'ellipse');
  track.setAttribute('cx', sportsPt.x);
  track.setAttribute('cy', sportsPt.y);
  track.setAttribute('rx', '95');
  track.setAttribute('ry', '52');
  track.setAttribute('class', 'svg-sports-track');
  layer.appendChild(track);

  const pitch = document.createElementNS('http://www.w3.org/2000/svg', 'ellipse');
  pitch.setAttribute('cx', sportsPt.x);
  pitch.setAttribute('cy', sportsPt.y);
  pitch.setAttribute('rx', '75');
  pitch.setAttribute('ry', '38');
  pitch.setAttribute('class', 'svg-sports-pitch');
  layer.appendChild(pitch);

  // 3. West & East Tree-lined Buffer Zones
  const westGreen = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  westGreen.setAttribute('d', 'M 70,160 Q 90,340 70,550 L 95,540 Q 115,340 95,170 Z');
  westGreen.setAttribute('class', 'svg-lawn');
  layer.appendChild(westGreen);

  const eastGreen = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  eastGreen.setAttribute('d', 'M 915,160 Q 890,340 915,550 L 890,540 Q 870,340 890,170 Z');
  eastGreen.setAttribute('class', 'svg-lawn');
  layer.appendChild(eastGreen);
}

function renderZonesLayer() {
  const layer = document.getElementById('layer-zones');
  if (!layer) return;
  layer.innerHTML = '';

  // 1. Visitor Parking Zone (Node 1: x=6.8, y=1.5)
  const pk1 = toSvgCoords(6.8, 1.5);
  const pkZone1 = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
  pkZone1.setAttribute('x', pk1.x - 38);
  pkZone1.setAttribute('y', pk1.y - 24);
  pkZone1.setAttribute('width', '76');
  pkZone1.setAttribute('height', '48');
  pkZone1.setAttribute('rx', '8');
  pkZone1.setAttribute('class', 'svg-parking-zone');
  layer.appendChild(pkZone1);

  // 2. Student 2-Wheeler Parking (Node 22: x=3.5, y=1.5)
  const pk2 = toSvgCoords(3.5, 1.5);
  const pkZone2 = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
  pkZone2.setAttribute('x', pk2.x - 38);
  pkZone2.setAttribute('y', pk2.y - 24);
  pkZone2.setAttribute('width', '76');
  pkZone2.setAttribute('height', '48');
  pkZone2.setAttribute('rx', '8');
  pkZone2.setAttribute('class', 'svg-parking-zone');
  layer.appendChild(pkZone2);

  // 3. Transport Bus Bay (Node 21: x=8.5, y=1.8)
  const bus = toSvgCoords(8.5, 1.8);
  const busZone = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
  busZone.setAttribute('x', bus.x - 42);
  busZone.setAttribute('y', bus.y - 24);
  busZone.setAttribute('width', '84');
  busZone.setAttribute('height', '48');
  busZone.setAttribute('rx', '8');
  busZone.setAttribute('class', 'svg-parking-zone');
  layer.appendChild(busZone);

  // 4. Open Air Theatre Stepped Semi-Circle (Node 25: x=5.2, y=5.2)
  const oat = toSvgCoords(5.2, 5.2);
  const oatArc = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
  oatArc.setAttribute('cx', oat.x);
  oatArc.setAttribute('cy', oat.y);
  oatArc.setAttribute('r', '26');
  oatArc.setAttribute('fill', '#1e1b4b');
  oatArc.setAttribute('stroke', '#818cf8');
  oatArc.setAttribute('stroke-width', '1.5');
  oatArc.setAttribute('stroke-dasharray', '3 3');
  layer.appendChild(oatArc);
}

function renderRoadsLayer() {
  const layer = document.getElementById('layer-roads');
  if (!layer) return;
  layer.innerHTML = '';

  State.edges.forEach(e => {
    const u = State.nodeMap.get(e.srcId);
    const v = State.nodeMap.get(e.destId);
    if (!u || !v) return;

    const pt1 = toSvgCoords(u.x, u.y);
    const pt2 = toSvgCoords(v.x, v.y);

    const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
    line.setAttribute('x1', pt1.x);
    line.setAttribute('y1', pt1.y);
    line.setAttribute('x2', pt2.x);
    line.setAttribute('y2', pt2.y);
    line.setAttribute('id', `road-${e.srcId}-${e.destId}`);

    let flowClass = 'flow-low';
    if (e.blocked) {
      flowClass = 'flow-blocked';
    } else if (State.showTraffic) {
      if (e.traffic === 'HIGH') flowClass = 'flow-high';
      else if (e.traffic === 'MEDIUM') flowClass = 'flow-med';
    }

    line.setAttribute('class', `svg-road-line ${flowClass}`);
    line.setAttribute('data-edge', `${e.srcId}-${e.destId}`);

    // Click on road quickly selects it in traffic/blockage control
    line.addEventListener('click', () => {
      const roadKey = `${e.srcId}-${e.destId}`;
      const revKey = `${e.destId}-${e.srcId}`;

      const tSelect = document.getElementById('select-traffic-road');
      const bSelect = document.getElementById('select-block-road');

      if (tSelect) {
        if (tSelect.querySelector(`option[value="${roadKey}"]`)) tSelect.value = roadKey;
        else if (tSelect.querySelector(`option[value="${revKey}"]`)) tSelect.value = revKey;
      }
      if (bSelect) {
        if (bSelect.querySelector(`option[value="${roadKey}"]`)) bSelect.value = roadKey;
        else if (bSelect.querySelector(`option[value="${revKey}"]`)) bSelect.value = revKey;
      }
      showToast(`Selected road: ${e.from} ⇄ ${e.to}`, 'info');
    });

    layer.appendChild(line);
  });
}

function renderBuildingsLayer() {
  const layer = document.getElementById('layer-buildings');
  if (!layer) return;
  layer.innerHTML = '';
  if (!State.showBuildings) return;

  State.nodes.forEach(node => {
    // Skip open lawn and raw stadium ground as extruded mass
    if (node.id === 6 || node.id === 18) return;

    const pt = toSvgCoords(node.x, node.y);
    const meta = BUILDING_DIMENSIONS[node.id] || { w: 60, h: 40, dz: 20, type: 'academic' };
    const dz = meta.dz || 18;

    const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
    g.setAttribute('class', 'svg-building-group');
    g.setAttribute('data-node-id', node.id);

    // 1. Ground Shadow Base
    const shadow = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
    shadow.setAttribute('x', pt.x - (meta.w / 2) + 4);
    shadow.setAttribute('y', pt.y - (meta.h / 2) + 8);
    shadow.setAttribute('width', meta.w);
    shadow.setAttribute('height', meta.h);
    shadow.setAttribute('rx', '8');
    shadow.setAttribute('fill', 'rgba(0,0,0,0.7)');
    g.appendChild(shadow);

    if (dz > 0) {
      // 2. Extruded South Wall Facade (Vertical facade from ground to roof)
      const bx1 = pt.x - (meta.w / 2), by1 = pt.y + (meta.h / 2);
      const bx2 = pt.x + (meta.w / 2), by2 = pt.y + (meta.h / 2);
      const southWall = document.createElementNS('http://www.w3.org/2000/svg', 'polygon');
      southWall.setAttribute('points', `${bx1},${by1} ${bx2},${by2} ${bx2},${by2 - dz} ${bx1},${by1 - dz}`);
      southWall.setAttribute('class', 'svg-wall-south');
      g.appendChild(southWall);

      // 3. Extruded East Wall Facade (Side facade adding architectural depth)
      const ex1 = pt.x + (meta.w / 2), ey1 = pt.y - (meta.h / 2);
      const ex2 = pt.x + (meta.w / 2), ey2 = pt.y + (meta.h / 2);
      const eastWall = document.createElementNS('http://www.w3.org/2000/svg', 'polygon');
      eastWall.setAttribute('points', `${ex1},${ey1} ${ex2},${ey2} ${ex2},${ey2 - dz} ${ex1},${ey1 - dz}`);
      eastWall.setAttribute('class', 'svg-wall-east');
      g.appendChild(eastWall);
    }

    // 4. Elevated 3D Roof Cap (Shifted up by dz)
    const roof = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
    roof.setAttribute('x', pt.x - (meta.w / 2));
    roof.setAttribute('y', pt.y - (meta.h / 2) - dz);
    roof.setAttribute('width', meta.w);
    roof.setAttribute('height', meta.h);
    roof.setAttribute('rx', '6');
    roof.setAttribute('class', `svg-building-rect svg-roof bldg-${meta.type}`);
    roof.setAttribute('stroke-width', '1.5');
    g.appendChild(roof);

    // 5. Roof Interior Inset Border & Glass Accent
    const roofInset = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
    roofInset.setAttribute('x', pt.x - (meta.w / 2) + 4);
    roofInset.setAttribute('y', pt.y - (meta.h / 2) - dz + 4);
    roofInset.setAttribute('width', meta.w - 8);
    roofInset.setAttribute('height', meta.h - 8);
    roofInset.setAttribute('rx', '4');
    roofInset.setAttribute('fill', 'url(#roof-glass-gradient)');
    roofInset.setAttribute('stroke', 'rgba(255,255,255,0.18)');
    roofInset.setAttribute('stroke-width', '0.8');
    roofInset.style.pointerEvents = 'none';
    g.appendChild(roofInset);

    // Interaction Handlers: Hover popover & Click
    g.addEventListener('mouseenter', (ev) => showBuildingPopover(node, ev));
    g.addEventListener('mouseleave', () => scheduleHidePopover());
    g.addEventListener('click', () => onNodeClicked(node.id));

    layer.appendChild(g);
  });
}

function renderNodesLayer() {
  const layer = document.getElementById('layer-nodes');
  if (!layer) return;
  layer.innerHTML = '';

  State.nodes.forEach(node => {
    const pt = toSvgCoords(node.x, node.y);
    const meta = BUILDING_DIMENSIONS[node.id];
    const dz = (meta && meta.dz) ? meta.dz : 0;

    const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
    g.setAttribute('class', 'svg-node-anchor');
    g.setAttribute('id', `node-anchor-${node.id}`);
    g.setAttribute('data-id', node.id);

    // Outer Selection Halo
    const halo = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
    halo.setAttribute('cx', pt.x);
    halo.setAttribute('cy', pt.y - dz);
    halo.setAttribute('r', '14');
    halo.setAttribute('fill', 'transparent');
    halo.setAttribute('class', 'node-halo');
    g.appendChild(halo);

    // Landmark Circle on elevated roof or ground
    const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
    circle.setAttribute('cx', pt.x);
    circle.setAttribute('cy', pt.y - dz);
    circle.setAttribute('r', '5.5');
    circle.setAttribute('fill', '#0284c7');
    circle.setAttribute('stroke', '#ffffff');
    circle.setAttribute('stroke-width', '1.5');
    circle.setAttribute('class', 'svg-node-circle');
    g.appendChild(circle);

    g.addEventListener('click', (e) => {
      e.stopPropagation();
      onNodeClicked(node.id);
    });

    layer.appendChild(g);
  });
}

function renderLabelsLayer() {
  const layer = document.getElementById('layer-labels');
  if (!layer) return;
  layer.innerHTML = '';
  if (!State.showLabels) return;

  State.nodes.forEach(node => {
    const pt = toSvgCoords(node.x, node.y);
    const meta = BUILDING_DIMENSIONS[node.id];
    const dz = (meta && meta.dz) ? meta.dz : 0;
    const labelText = meta ? meta.label : node.name;

    const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    text.setAttribute('x', pt.x);
    text.setAttribute('y', pt.y - dz - 10);
    text.setAttribute('class', 'svg-landmark-text');
    text.textContent = labelText;
    layer.appendChild(text);
  });
}

function updateHighlightedNodes() {
  const srcSelect = document.getElementById('select-source');
  const destSelect = document.getElementById('select-dest');

  const srcId = srcSelect ? Number(srcSelect.value) : -1;
  const destId = destSelect ? Number(destSelect.value) : -1;

  document.querySelectorAll('.svg-node-circle').forEach(el => {
    el.classList.remove('node-selected-src', 'node-selected-dest');
  });

  if (!isNaN(srcId) && srcId >= 0) {
    const srcEl = document.querySelector(`#node-anchor-${srcId} .svg-node-circle`);
    if (srcEl) srcEl.classList.add('node-selected-src');
  }

  if (!isNaN(destId) && destId >= 0) {
    const destEl = document.querySelector(`#node-anchor-${destId} .svg-node-circle`);
    if (destEl) destEl.classList.add('node-selected-dest');
  }
}

// =============================================================================
// GOOGLE EARTH 3D CAMERA & CINEMATIC FLY-TO ANIMATION
// =============================================================================

function applySvgTransform() {
  const svg = document.getElementById('cgc-campus-svg');
  if (!svg) return;

  const pitch = State.is3D ? State.pitch : 0;
  const bearing = State.is3D ? State.bearing : 0;

  svg.style.transform = `translate(${State.panX}px, ${State.panY}px) scale(${State.zoomLevel}) rotateX(${pitch}deg) rotateZ(${bearing}deg)`;

  // Update Compass needle in real-time
  const needle = document.getElementById('compass-needle');
  if (needle) {
    needle.style.transform = `rotate(${-bearing}deg)`;
  }
}

function toggle3DMode() {
  State.is3D = !State.is3D;
  const btn = document.getElementById('btn-toggle-3d');
  if (btn) btn.classList.toggle('active', State.is3D);

  if (State.is3D) {
    State.pitch = 48;
    State.bearing = -14;
    showToast('🌍 Google Earth 3D Perspective Mode Active', 'info');
  } else {
    if (State.isOrbiting) toggleOrbitCampus();
    State.pitch = 0;
    State.bearing = 0;
    showToast('2D Plan View Active', 'info');
  }
  applySvgTransform();
}

function toggleOrbitCampus() {
  State.isOrbiting = !State.isOrbiting;
  const btn = document.getElementById('btn-orbit-campus');
  if (btn) btn.classList.toggle('active', State.isOrbiting);

  if (State.isOrbiting) {
    if (!State.is3D) {
      State.is3D = true;
      const btn3d = document.getElementById('btn-toggle-3d');
      if (btn3d) btn3d.classList.add('active');
    }
    if (State.pitch < 32) State.pitch = 48;
    showToast('🛰️ Cinematic 360° Aerial Orbit Flyover Active', 'info');
    runOrbitLoop();
  } else {
    cancelAnimationFrame(State.orbitRafId);
    showToast('Orbit Flyover Paused', 'info');
  }
}

function runOrbitLoop() {
  if (!State.isOrbiting) return;
  State.bearing = (State.bearing + 0.32) % 360;
  applySvgTransform();
  State.orbitRafId = requestAnimationFrame(runOrbitLoop);
}

/**
 * Google Earth Cinematic Fly-To camera animation.
 * Smoothly swoops from current camera view to target coordinates with altitude arc.
 */
function flyToCoords(targetX, targetY, targetZoom = 1.3, targetPitch = 48) {
  if (State.isOrbiting) toggleOrbitCampus();

  const centerSvgX = SVG_CONFIG.viewWidth / 2;
  const centerSvgY = SVG_CONFIG.viewHeight / 2;

  const destPanX = (centerSvgX - targetX) * 0.75;
  const destPanY = (centerSvgY - targetY) * 0.75;

  const startPanX = State.panX;
  const startPanY = State.panY;
  const startZoom = State.zoomLevel;
  const startPitch = State.pitch;

  const duration = 1250;
  const startTime = performance.now();

  function easeInOutCubic(t) {
    return t < 0.5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2;
  }

  function flightFrame(now) {
    const elapsed = now - startTime;
    const progress = Math.min(1, elapsed / duration);
    const ease = easeInOutCubic(progress);

    // Altitude curvature: zoom pulls back slightly at mid-flight
    const arcDelta = Math.sin(progress * Math.PI) * 0.18;

    State.panX = startPanX + (destPanX - startPanX) * ease;
    State.panY = startPanY + (destPanY - startPanY) * ease;
    State.zoomLevel = Math.max(0.7, (startZoom + (targetZoom - startZoom) * ease) - arcDelta);
    if (State.is3D) {
      State.pitch = startPitch + (targetPitch - startPitch) * ease;
    }

    applySvgTransform();

    if (progress < 1) {
      requestAnimationFrame(flightFrame);
    } else {
      spawnFlyToRadarBeacon(targetX, targetY);
    }
  }

  requestAnimationFrame(flightFrame);
}

function spawnFlyToRadarBeacon(x, y) {
  const container = document.getElementById('layer-waypoint-runner');
  if (!container) return;

  const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
  g.setAttribute('class', 'flyto-beacon-group');

  const ring1 = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
  ring1.setAttribute('cx', x);
  ring1.setAttribute('cy', y);
  ring1.setAttribute('r', '10');
  ring1.setAttribute('class', 'flyto-radar-ring');
  g.appendChild(ring1);

  const ring2 = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
  ring2.setAttribute('cx', x);
  ring2.setAttribute('cy', y);
  ring2.setAttribute('r', '10');
  ring2.setAttribute('class', 'flyto-radar-ring flyto-radar-ring-2');
  g.appendChild(ring2);

  container.appendChild(g);

  // Auto clean-up after 3.8s
  setTimeout(() => g.remove(), 3800);
}

// =============================================================================
// INTERACTIVE POPOVER CARD
// =============================================================================

// =============================================================================
// INTERACTIVE POPOVER CARD & POINT SELECTION
// =============================================================================

let popoverTimeout = null;

function showBuildingPopover(node, ev) {
  clearTimeout(popoverTimeout);
  const popover = document.getElementById('building-popover');
  if (!popover) return;

  const catEl = document.getElementById('popover-category');
  const titleEl = document.getElementById('popover-title');
  const descEl = document.getElementById('popover-desc');
  const btnSrc = document.getElementById('btn-popover-src');
  const btnDest = document.getElementById('btn-popover-dest');
  const btnClear = document.getElementById('btn-popover-clear');

  if (catEl) catEl.textContent = (node.type || 'CAMPUS LANDMARK').toUpperCase();
  if (titleEl) titleEl.textContent = node.name;
  if (descEl) descEl.textContent = node.description || 'Key facility within CGC Landran campus.';

  const container = document.getElementById('map-container');
  const rect = container.getBoundingClientRect();
  const pt = toSvgCoords(node.x, node.y);

  const posX = (pt.x * (rect.width / SVG_CONFIG.viewWidth) * State.zoomLevel) + State.panX;
  const posY = (pt.y * (rect.height / SVG_CONFIG.viewHeight) * State.zoomLevel) + State.panY;

  popover.style.left = `${Math.min(rect.width - 250, Math.max(20, posX - 100))}px`;
  popover.style.top = `${Math.min(rect.height - 180, Math.max(20, posY - 120))}px`;
  popover.classList.remove('hidden');

  const srcSelect = document.getElementById('select-source');
  const destSelect = document.getElementById('select-dest');

  const isCurrentSrc = srcSelect && Number(srcSelect.value) === node.id;
  const isCurrentDest = destSelect && Number(destSelect.value) === node.id;

  if (btnClear) {
    btnClear.style.display = (isCurrentSrc || isCurrentDest) ? 'inline-block' : 'none';
  }

  btnSrc.onclick = () => {
    if (srcSelect) srcSelect.value = node.id;
    if (destSelect && Number(destSelect.value) === node.id) destSelect.value = "";
    if (State.activeRoute) {
      exitNavigation(false);
    }
    updateHighlightedNodes();
    popover.classList.add('hidden');
    showToast(`🟢 Starting location set to: ${node.name}. Now select where to go.`, 'info');
  };

  btnDest.onclick = () => {
    if (destSelect) destSelect.value = node.id;
    updateHighlightedNodes();
    popover.classList.add('hidden');
    showToast(`🔵 Destination set to: ${node.name}`, 'info');
    if (srcSelect && srcSelect.value && Number(srcSelect.value) !== node.id) {
      findOptimalRoute();
    }
  };

  if (btnClear) {
    btnClear.onclick = () => {
      if (srcSelect && Number(srcSelect.value) === node.id) srcSelect.value = "";
      if (destSelect && Number(destSelect.value) === node.id) destSelect.value = "";
      if (State.activeRoute) exitNavigation(false);
      updateHighlightedNodes();
      popover.classList.add('hidden');
      showToast(`Deselected ${node.name}.`, 'info');
    };
  }

  popover.onmouseenter = () => clearTimeout(popoverTimeout);
  popover.onmouseleave = () => popover.classList.add('hidden');
}

function scheduleHidePopover() {
  popoverTimeout = setTimeout(() => {
    const popover = document.getElementById('building-popover');
    if (popover) popover.classList.add('hidden');
  }, 400);
}

function onNodeClicked(nodeId) {
  const node = State.nodeMap.get(nodeId);
  if (!node) return;

  const pt = toSvgCoords(node.x, node.y);
  flyToCoords(pt.x, pt.y);

  const srcSelect = document.getElementById('select-source');
  const destSelect = document.getElementById('select-dest');

  // If no starting location is set, immediately assign as start
  if (!srcSelect || !srcSelect.value || srcSelect.value === "") {
    if (srcSelect) srcSelect.value = nodeId;
    updateHighlightedNodes();
    showToast(`🟢 Set Start: ${node.name}. Click destination or use popover to navigate.`, 'info');
    showBuildingPopover(node);
    return;
  }

  // If clicking on the already selected start point, open popover allowing them to clear or change
  if (Number(srcSelect.value) === nodeId) {
    showBuildingPopover(node);
    return;
  }

  // If starting location is set and no destination is set yet, assign destination & route
  if (!destSelect.value || destSelect.value === "") {
    destSelect.value = nodeId;
    updateHighlightedNodes();
    showToast(`🔵 Destination set to: ${node.name}`, 'info');
    findOptimalRoute();
    showBuildingPopover(node);
    return;
  }

  // If both start and destination are already set, open popover so user can easily choose
  // "Start Here" (to change starting point to here) or "Navigate Here" (to change destination)!
  showBuildingPopover(node);
}

function clearStartingLocation() {
  const srcSelect = document.getElementById('select-source');
  if (srcSelect) srcSelect.value = "";
  if (State.activeRoute) exitNavigation(false);
  updateHighlightedNodes();
  showToast('Starting location cleared. Click any campus landmark to set a new start point.', 'info');
}

function clearDestination() {
  const destSelect = document.getElementById('select-dest');
  if (destSelect) destSelect.value = "";
  if (State.activeRoute) exitNavigation(false);
  updateHighlightedNodes();
  showToast('Destination cleared.', 'info');
}

function exitNavigation(showNotification = true) {
  // 1. Cancel running simulation
  clearInterval(State.simNavInterval);
  State.simNavInterval = null;
  State.simNavStep = 0;

  // 2. Hide HUD banner
  const hud = document.getElementById('sim-nav-hud');
  if (hud) hud.classList.add('hidden');

  // 3. Clear active route polyline from map
  const routeLayer = document.getElementById('layer-active-route');
  if (routeLayer) routeLayer.innerHTML = '';

  // 4. Clear waypoint runner avatar
  const runnerLayer = document.getElementById('layer-waypoint-runner');
  if (runnerLayer) runnerLayer.innerHTML = '';

  // 5. Reset active route state
  State.activeRoute = null;

  // 6. Reset route state pill
  const pill = document.getElementById('route-state-pill');
  if (pill) {
    pill.className = 'state-pill ready';
    pill.textContent = 'Awaiting Selection';
  }

  // 7. Reset KPIs
  const kpiTime = document.getElementById('kpi-time');
  const kpiDist = document.getElementById('kpi-dist');
  const kpiNodes = document.getElementById('kpi-nodes');
  const kpiCpu = document.getElementById('kpi-cpu');
  if (kpiTime) kpiTime.innerHTML = `-- <small>min</small>`;
  if (kpiDist) kpiDist.innerHTML = `-- <small>km</small>`;
  if (kpiNodes) kpiNodes.textContent = '--';
  if (kpiCpu) kpiCpu.innerHTML = `-- <small>ms</small>`;

  // 8. Reset lists
  const listEl = document.getElementById('route-sequence-list');
  if (listEl) {
    listEl.innerHTML = '<div class="step-empty">Select start and destination to view recommended route.</div>';
  }
  const turnsEl = document.getElementById('turn-steps-container');
  if (turnsEl) {
    turnsEl.innerHTML = '<div class="step-empty">No active route computed.</div>';
  }
  const altEl = document.getElementById('alt-bypass-content');
  if (altEl) {
    altEl.innerHTML = '<p class="card-hint">Secondary walking corridor will be shown if distinct routes exist across campus.</p>';
  }

  // 9. Reset destination selection so user can pick where to go from start point
  const destSelect = document.getElementById('select-dest');
  if (destSelect) destSelect.value = "";

  // 10. Update highlighted nodes
  updateHighlightedNodes();

  if (showNotification) {
    showToast('Navigation exited. Route cleared.', 'info');
  }
}

// =============================================================================
// ROUTING ENGINE & API CALLS
// =============================================================================

async function findOptimalRoute() {
  const srcSelect = document.getElementById('select-source');
  const destSelect = document.getElementById('select-dest');
  const algoSelect = document.getElementById('select-algorithm');
  const modeSelect = document.getElementById('select-route-mode');

  const srcVal = srcSelect ? srcSelect.value : "";
  const destVal = destSelect ? destSelect.value : "";
  const algoVal = algoSelect ? algoSelect.value : "astar";
  const modeVal = modeSelect ? modeSelect.value : "walking";

  if (!srcVal || !destVal) {
    showToast('Please select both a Start location and Destination.', 'warning');
    return;
  }

  if (srcVal === destVal) {
    showToast('Start and Destination are the same location! Distance: 0 km.', 'info');
    return;
  }

  const pill = document.getElementById('route-state-pill');
  if (pill) {
    pill.className = 'state-pill computing';
    pill.textContent = 'Computing Optimal Route...';
  }

  try {
    const res = await fetch('/api/route', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        source: srcVal,
        destination: destVal,
        algorithm: algoVal,
        mode: modeVal
      })
    });

    const data = await res.json();

    if (!data.success) {
      renderRouteUnavailable(data.error);
      return;
    }

    State.activeRoute = data;
    renderActiveRoute(data);
    updateIntelligencePanel(data);

    if (pill) {
      pill.className = 'state-pill ready';
      pill.textContent = `Route Optimized (${data.algorithm})`;
    }

    // Google Earth Fly-To Destination with altitude swoop
    const destNode = State.nodeMap.get(data.destination_id);
    if (destNode) {
      const pt = toSvgCoords(destNode.x, destNode.y);
      flyToCoords(pt.x, pt.y, 1.35, 52);
    }

    showToast(`Found optimal route via ${data.algorithm} in ${data.execution_time_ms} ms!`, 'success');
  } catch (err) {
    console.error('Route calculation error:', err);
    showToast('Failed to calculate route. Ensure C++ server is active.', 'danger');
  }
}

function renderActiveRoute(routeData) {
  const routeLayer = document.getElementById('layer-active-route');
  const runnerLayer = document.getElementById('layer-waypoint-runner');
  if (!routeLayer) return;

  routeLayer.innerHTML = '';
  if (runnerLayer) runnerLayer.innerHTML = '';

  const pathIds = routeData.route_ids || [];
  if (pathIds.length < 2) return;

  // Build SVG polyline points string
  const points = pathIds.map(id => {
    const node = State.nodeMap.get(id);
    if (!node) return '0,0';
    const pt = toSvgCoords(node.x, node.y);
    return `${pt.x},${pt.y}`;
  }).join(' ');

  const polyline = document.createElementNS('http://www.w3.org/2000/svg', 'polyline');
  polyline.setAttribute('points', points);
  polyline.setAttribute('class', 'svg-active-route-path');
  routeLayer.appendChild(polyline);

  // Render glowing waypoint runner on the start node
  if (runnerLayer && pathIds.length > 0) {
    const startNode = State.nodeMap.get(pathIds[0]);
    if (startNode) {
      const startPt = toSvgCoords(startNode.x, startNode.y);
      const runner = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
      runner.setAttribute('cx', startPt.x);
      runner.setAttribute('cy', startPt.y);
      runner.setAttribute('r', '7.5');
      runner.setAttribute('fill', '#00f2fe');
      runner.setAttribute('stroke', '#ffffff');
      runner.setAttribute('stroke-width', '2.5');
      runner.setAttribute('id', 'active-runner-avatar');
      runnerLayer.appendChild(runner);
    }
  }

  updateHighlightedNodes();
}

function renderRouteUnavailable(errorMsg) {
  const routeLayer = document.getElementById('layer-active-route');
  const runnerLayer = document.getElementById('layer-waypoint-runner');
  if (routeLayer) routeLayer.innerHTML = '';
  if (runnerLayer) runnerLayer.innerHTML = '';

  const pill = document.getElementById('route-state-pill');
  if (pill) {
    pill.className = 'state-pill blocked';
    pill.textContent = 'No Route Available';
  }

  const listEl = document.getElementById('route-sequence-list');
  if (listEl) {
    listEl.innerHTML = `
      <div class="empty-state-card">
        <h4 style="color: var(--cgc-crimson); margin-bottom: 6px;">⛔ PATH DISCONNECTED</h4>
        <p style="font-size: 0.85rem; color: var(--text-muted);">${errorMsg || 'Active road closures have completely disconnected the chosen landmarks.'}</p>
        <button id="btn-empty-reset" class="btn btn-micro-action danger" style="margin-top: 10px;">Reset Road Network</button>
      </div>
    `;
    const btn = document.getElementById('btn-empty-reset');
    if (btn) btn.onclick = resetRoadNetwork;
  }

  showToast('No route available due to blocked road segments.', 'danger');
}

function updateIntelligencePanel(routeData) {
  const kpiTime = document.getElementById('kpi-time');
  const kpiDist = document.getElementById('kpi-dist');
  const kpiNodes = document.getElementById('kpi-nodes');
  const kpiCpu = document.getElementById('kpi-cpu');

  if (kpiTime) kpiTime.innerHTML = `${routeData.travel_time_min} <small>min</small>`;
  if (kpiDist) kpiDist.innerHTML = `${routeData.distance_km} <small>km</small>`;
  if (kpiNodes) kpiNodes.textContent = routeData.nodes_explored;
  if (kpiCpu) kpiCpu.innerHTML = `${routeData.execution_time_ms} <small>ms</small>`;

  // Render Step-by-Step Route Sequence
  const listEl = document.getElementById('route-sequence-list');
  if (listEl) {
    listEl.innerHTML = '';
    const route = routeData.route || [];

    route.forEach((locName, idx) => {
      const step = document.createElement('div');
      step.className = 'route-step-item';

      const isFirst = (idx === 0);
      const isLast = (idx === route.length - 1);
      const markerType = isFirst ? 'step-src' : isLast ? 'step-dest' : 'step-mid';

      step.innerHTML = `
        <span class="step-marker ${markerType}">${idx + 1}</span>
        <div class="step-content">
          <span class="step-title">${locName}</span>
          <span class="step-desc">${isFirst ? 'Origin Landmark' : isLast ? 'Final Destination' : 'Transit Corridor'}</span>
        </div>
      `;
      listEl.appendChild(step);
    });
  }

  // Render Turn-by-Turn Segment Cards
  const turnsEl = document.getElementById('turn-steps-container');
  if (turnsEl) {
    turnsEl.innerHTML = '';
    const segments = routeData.segments || [];

    segments.forEach((seg, idx) => {
      const card = document.createElement('div');
      card.className = 'turn-card';
      const trafficBadge = seg.traffic !== 'LOW' ? `<span class="traffic-tag ${seg.traffic.toLowerCase()}">${seg.traffic} TRAFFIC</span>` : '';

      card.innerHTML = `
        <div class="turn-header">
          <span class="turn-num">Leg ${idx + 1}</span>
          <span class="turn-dist">${seg.distance_km} km • ${seg.travel_time_min} min</span>
        </div>
        <div class="turn-name">Follow pathway from <strong>${seg.from}</strong> to <strong>${seg.to}</strong></div>
        ${trafficBadge}
      `;
      turnsEl.appendChild(card);
    });
  }

  // Render Alternative Route / Bypass Option
  const altEl = document.getElementById('alt-bypass-content');
  if (altEl) {
    if (routeData.alternative_route && routeData.alternative_route.found) {
      const alt = routeData.alternative_route;
      altEl.innerHTML = `
        <div class="alt-route-box">
          <div class="alt-route-header">
            <strong>Bypass Alternative</strong>
            <span>${alt.distance_km} km • ${alt.travel_time_min} min</span>
          </div>
          <p style="font-size: 0.78rem; color: var(--text-muted); margin-bottom: 6px;">${alt.rationale || 'Secondary corridor avoiding primary bottleneck.'}</p>
          <div style="font-size: 0.8rem; color: var(--cgc-azure);">${alt.route.join(' → ')}</div>
        </div>
      `;
    } else {
      altEl.innerHTML = `<span style="font-size: 0.8rem; color: var(--text-dim);">No distinct alternative bypass is currently necessary.</span>`;
    }
  }
}

// =============================================================================
// DYNAMIC TRAFFIC & ROAD BLOCKAGE SIMULATION
// =============================================================================

async function updateTrafficSimulation(level) {
  const roadSelect = document.getElementById('select-traffic-road');
  if (!roadSelect || !roadSelect.value) {
    showToast('Please select a road segment first.', 'warning');
    return;
  }

  const [src, dest] = roadSelect.value.split('-');

  try {
    const res = await fetch('/api/traffic', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        src: Number(src),
        dest: Number(dest),
        level: level
      })
    });

    const data = await res.json();
    if (data.success) {
      showToast(`Simulated traffic updated to ${level} on selected corridor!`, 'info');
      await fetchCampusData();
      if (State.activeRoute) findOptimalRoute();
    }
  } catch (err) {
    console.error('Traffic update error:', err);
  }
}

async function blockSelectedRoad() {
  const roadSelect = document.getElementById('select-block-road');
  if (!roadSelect || !roadSelect.value) {
    showToast('Please select a pathway to block.', 'warning');
    return;
  }

  const [src, dest] = roadSelect.value.split('-');

  try {
    const res = await fetch('/api/road/block', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ src: Number(src), dest: Number(dest) })
    });

    const data = await res.json();
    if (data.success) {
      showToast(`Pathway blocked: Dynamic graph rerouting engaged!`, 'danger');
      await fetchCampusData();
      if (State.activeRoute) findOptimalRoute();
    }
  } catch (err) {
    console.error('Block road error:', err);
  }
}

async function unblockSelectedRoad() {
  const roadSelect = document.getElementById('select-block-road');
  if (!roadSelect || !roadSelect.value) {
    showToast('Please select a pathway to unblock.', 'warning');
    return;
  }

  const [src, dest] = roadSelect.value.split('-');

  try {
    const res = await fetch('/api/road/unblock', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ src: Number(src), dest: Number(dest) })
    });

    const data = await res.json();
    if (data.success) {
      showToast(`Pathway restored to open service!`, 'success');
      await fetchCampusData();
      if (State.activeRoute) findOptimalRoute();
    }
  } catch (err) {
    console.error('Unblock road error:', err);
  }
}

async function resetRoadNetwork() {
  try {
    const res = await fetch('/api/reset', { method: 'POST' });
    const data = await res.json();
    if (data.success) {
      showToast('All closures and simulated congestion reset to default!', 'success');
      await fetchCampusData();
      if (State.activeRoute) findOptimalRoute();
    }
  } catch (err) {
    console.error('Reset network error:', err);
  }
}

// =============================================================================
// GRAPH TRAVERSAL ANIMATION (BFS & DFS)
// =============================================================================

async function runGraphTraversal(algorithm) {
  const travSelect = document.getElementById('select-traversal-start');
  const startId = travSelect ? Number(travSelect.value) : 0;

  try {
    const res = await fetch('/api/traversal', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        start_node: startId,
        algorithm: algorithm
      })
    });

    const data = await res.json();
    if (!data.success) {
      showToast('Traversal failed: ' + data.error, 'danger');
      return;
    }

    showToast(`Animating ${algorithm.toUpperCase()} traversal from [${data.start_node}]...`, 'info');
    animateTraversal(data.traversal_order || data.traversal_ids, algorithm);
  } catch (err) {
    console.error('Traversal error:', err);
  }
}

function animateTraversal(nodeIds, algorithm) {
  const seqBox = document.getElementById('traversal-sequence-box');
  if (seqBox) {
    seqBox.classList.remove('hidden');
    seqBox.innerHTML = `<strong>${algorithm.toUpperCase()} Order (${nodeIds.length} nodes):</strong><br>`;
  }

  // Clear previous route highlight temporarily
  const routeLayer = document.getElementById('layer-active-route');
  if (routeLayer) routeLayer.innerHTML = '';

  nodeIds.forEach((id, idx) => {
    setTimeout(() => {
      const nodeEl = document.querySelector(`#node-anchor-${id} .svg-node-circle`);
      if (nodeEl) {
        nodeEl.style.fill = algorithm === 'bfs' ? '#00f2fe' : '#a855f7';
        nodeEl.setAttribute('r', '9');
        setTimeout(() => {
          nodeEl.setAttribute('r', '5.5');
          nodeEl.style.fill = '#0284c7';
        }, 1200);
      }

      if (seqBox) {
        const node = State.nodeMap.get(id);
        const name = node ? node.name : id;
        seqBox.innerHTML += `<span class="traversal-chip">${idx + 1}. ${name}</span> `;
      }
    }, idx * 250);
  });
}

// =============================================================================
// SIMULATED LIVE NAVIGATION HUD
// =============================================================================

function startSimulatedNavigation() {
  if (!State.activeRoute || !State.activeRoute.route_ids || State.activeRoute.route_ids.length < 2) {
    showToast('Please find an optimal route first to start simulated navigation.', 'warning');
    return;
  }

  const hud = document.getElementById('sim-nav-hud');
  if (hud) hud.classList.remove('hidden');

  State.simNavStep = 0;
  clearInterval(State.simNavInterval);

  showToast('Starting Simulated Step-by-Step Campus Navigation...', 'info');
  updateSimNavHUD();

  State.simNavInterval = setInterval(() => {
    State.simNavStep++;
    const pathIds = State.activeRoute.route_ids;

    if (State.simNavStep >= pathIds.length) {
      clearInterval(State.simNavInterval);
      showToast('You have arrived at your destination!', 'success');
      setTimeout(stopSimulatedNavigation, 2500);
      return;
    }

    updateSimNavHUD();
  }, 2200);
}

function updateSimNavHUD() {
  const pathIds = State.activeRoute.route_ids;
  const currId = pathIds[State.simNavStep];
  const nextId = pathIds[Math.min(State.simNavStep + 1, pathIds.length - 1)];

  const currNode = State.nodeMap.get(currId);
  const nextNode = State.nodeMap.get(nextId);

  const currEl = document.getElementById('hud-current-node');
  const nextEl = document.getElementById('hud-next-node');
  const distEl = document.getElementById('hud-rem-dist');
  const timeEl = document.getElementById('hud-rem-time');

  if (currEl && currNode) currEl.textContent = currNode.name;
  if (nextEl && nextNode) nextEl.textContent = currId === nextId ? 'Arrived at Destination' : nextNode.name;

  // Move runner avatar along the SVG
  if (currNode) {
    const pt = toSvgCoords(currNode.x, currNode.y);
    const runner = document.getElementById('active-runner-avatar');
    if (runner) {
      runner.setAttribute('cx', pt.x);
      runner.setAttribute('cy', pt.y);
      runner.setAttribute('r', '9');
    }
  }

  const totalSteps = pathIds.length - 1;
  const remFraction = Math.max(0, 1 - (State.simNavStep / totalSteps));
  const remDist = (State.activeRoute.distance_km * remFraction).toFixed(2);
  const remTime = (State.activeRoute.travel_time_min * remFraction).toFixed(1);

  if (distEl) distEl.textContent = `${remDist} km`;
  if (timeEl) timeEl.textContent = `${remTime} min`;
}

function stopSimulatedNavigation() {
  clearInterval(State.simNavInterval);
  State.simNavInterval = null;
  const hud = document.getElementById('sim-nav-hud');
  if (hud) hud.classList.add('hidden');
  const runner = document.getElementById('active-runner-avatar');
  if (runner) runner.remove();
}

// =============================================================================
// MODAL PANELS (ALGORITHM LAB, DSA INSPECTOR, VIVA GUIDE)
// =============================================================================

async function openAlgorithmLab() {
  const modal = document.getElementById('modal-algo-lab');
  if (!modal) return;
  modal.classList.remove('hidden');

  const srcSelect = document.getElementById('select-source');
  const destSelect = document.getElementById('select-dest');
  const srcId = srcSelect ? srcSelect.value : "0";
  const destId = destSelect ? destSelect.value : "4";

  try {
    // Run Dijkstra
    const dRes = await fetch('/api/route', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ source: srcId, destination: destId, algorithm: 'dijkstra', mode: 'walking' })
    }).then(r => r.json());

    // Run A*
    const aRes = await fetch('/api/route', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ source: srcId, destination: destId, algorithm: 'astar', mode: 'walking' })
    }).then(r => r.json());

    document.getElementById('lab-dijkstra-nodes').textContent = dRes.nodes_explored || '--';
    document.getElementById('lab-astar-nodes').textContent = aRes.nodes_explored || '--';
    document.getElementById('lab-dijkstra-time').textContent = (dRes.execution_time_ms || 0) + ' ms';
    document.getElementById('lab-astar-time').textContent = (aRes.execution_time_ms || 0) + ' ms';
    document.getElementById('lab-dijkstra-dist').textContent = (dRes.distance_km || 0) + ' km';
    document.getElementById('lab-astar-dist').textContent = (aRes.distance_km || 0) + ' km';
  } catch (err) {
    console.error('Algorithm lab error:', err);
  }
}

function openDsaInspector() {
  const modal = document.getElementById('modal-inspector');
  if (!modal) return;
  modal.classList.remove('hidden');

  const inspectorContent = document.getElementById('inspector-dynamic-content');
  if (!inspectorContent) return;

  if (!State.activeRoute) {
    inspectorContent.innerHTML = `<div class="empty-state-card"><p>Please calculate an optimal route first to view runtime priority queue states and mathematical relaxation.</p></div>`;
    return;
  }

  const r = State.activeRoute;
  inspectorContent.innerHTML = `
    <div class="inspector-grid">
      <div class="inspector-card">
        <h4>Runtime Min-Heap & Relaxation</h4>
        <div class="code-dsa-block">
Algorithm: ${r.algorithm}
Heuristic: Admissible Straight-Line Euclidean Bound (h(n) ≤ h*(n))
Explored Graph Vertices: ${r.nodes_explored}
Total Path Vertices in Tree: ${(r.route_ids || []).length}
Execution CPU Time: ${r.execution_time_ms} ms
        </div>
      </div>
      <div class="inspector-card">
        <h4>Reconstructed Path Vertex Vector</h4>
        <div class="code-dsa-block">
${(r.route || []).map((name, i) => `[${i}] Vertex: ${name} (ID: ${r.route_ids[i]})`).join('\n')}
        </div>
      </div>
    </div>
  `;
}

function openVivaGuide() {
  const modal = document.getElementById('modal-viva');
  if (modal) modal.classList.remove('hidden');
}

function closeAllModals() {
  document.querySelectorAll('.modal-layer').forEach(m => m.classList.add('hidden'));
}

// =============================================================================
// ZOOM & 3D CAMERA CONTROLS
// =============================================================================

function zoomIn() {
  State.zoomLevel = Math.min(3.2, State.zoomLevel + 0.2);
  applySvgTransform();
}

function zoomOut() {
  State.zoomLevel = Math.max(0.65, State.zoomLevel - 0.2);
  applySvgTransform();
}

function resetZoom() {
  State.zoomLevel = 1.05;
  State.panX = 0;
  State.panY = 20;
  State.pitch = State.is3D ? 46 : 0;
  State.bearing = State.is3D ? -12 : 0;
  applySvgTransform();
}

function tiltUp() {
  State.is3D = true;
  State.pitch = Math.min(65, State.pitch + 8);
  const btn3d = document.getElementById('btn-toggle-3d');
  if (btn3d) btn3d.classList.add('active');
  applySvgTransform();
}

function tiltDown() {
  State.pitch = Math.max(0, State.pitch - 8);
  if (State.pitch === 0) {
    State.is3D = false;
    const btn3d = document.getElementById('btn-toggle-3d');
    if (btn3d) btn3d.classList.remove('active');
  }
  applySvgTransform();
}

function rotateLeft() {
  State.bearing = (State.bearing - 15) % 360;
  applySvgTransform();
}

function rotateRight() {
  State.bearing = (State.bearing + 15) % 360;
  applySvgTransform();
}

function resetNorth() {
  State.bearing = 0;
  applySvgTransform();
  showToast('Orientation reset to North (0° Azimuth)', 'info');
}

// =============================================================================
// INITIALIZATION & EVENT LISTENERS
// =============================================================================

document.addEventListener('DOMContentLoaded', () => {
  // 1. Fetch initial campus topology and stats (front page loads immediately)
  fetchCampusData();

  // 2. Quick Landmark Access Chips
  document.querySelectorAll('.chip-landmark').forEach(chip => {
    chip.addEventListener('click', () => {
      const destId = chip.getAttribute('data-id');
      const destSelect = document.getElementById('select-dest');
      if (destSelect && destId) {
        destSelect.value = destId;
        updateHighlightedNodes();
        showToast(`Destination selected: ${chip.textContent.trim()}`, 'info');

        const node = State.nodeMap.get(Number(destId));
        if (node) {
          const pt = toSvgCoords(node.x, node.y);
          flyToCoords(pt.x, pt.y);
        }

        findOptimalRoute();
      }
    });
  });

  // 3. Primary Actions
  const btnFindRoute = document.getElementById('btn-find-route');
  if (btnFindRoute) btnFindRoute.addEventListener('click', findOptimalRoute);

  const btnRecalc = document.getElementById('btn-recalculate-route');
  if (btnRecalc) btnRecalc.addEventListener('click', findOptimalRoute);

  const btnSwap = document.getElementById('btn-swap-locations');
  if (btnSwap) {
    btnSwap.addEventListener('click', () => {
      const s = document.getElementById('select-source');
      const d = document.getElementById('select-dest');
      if (s && d) {
        const tmp = s.value;
        s.value = d.value;
        d.value = tmp;
        updateHighlightedNodes();
        if (s.value && d.value) findOptimalRoute();
      }
    });
  }

  // 4. Select dropdown changes
  const selectSrc = document.getElementById('select-source');
  const selectDest = document.getElementById('select-dest');
  if (selectSrc) selectSrc.addEventListener('change', updateHighlightedNodes);
  if (selectDest) selectDest.addEventListener('change', () => {
    updateHighlightedNodes();
    findOptimalRoute();
  });

  // 5. Algorithm selection badge sync
  const selectAlgo = document.getElementById('select-algorithm');
  const badgeAlgo = document.getElementById('badge-active-algo');
  if (selectAlgo && badgeAlgo) {
    selectAlgo.addEventListener('change', () => {
      badgeAlgo.textContent = selectAlgo.value === 'astar' ? 'A* Heuristic' : 'Dijkstra Min-Heap';
      if (State.activeRoute) findOptimalRoute();
    });
  }

  // 6. Traffic Simulation Controls
  document.querySelectorAll('.btn-traffic').forEach(btn => {
    btn.addEventListener('click', () => {
      const level = btn.getAttribute('data-level');
      if (level) updateTrafficSimulation(level);
    });
  });

  // 7. Road Block / Unblock Controls
  const btnBlock = document.getElementById('btn-block-path');
  const btnUnblock = document.getElementById('btn-unblock-path');
  const btnReset = document.getElementById('btn-reset-graph');
  if (btnBlock) btnBlock.addEventListener('click', blockSelectedRoad);
  if (btnUnblock) btnUnblock.addEventListener('click', unblockSelectedRoad);
  if (btnReset) btnReset.addEventListener('click', resetRoadNetwork);

  // 8. Traversals (BFS / DFS)
  const btnBfs = document.getElementById('btn-run-bfs');
  const btnDfs = document.getElementById('btn-run-dfs');
  if (btnBfs) btnBfs.addEventListener('click', () => runGraphTraversal('bfs'));
  if (btnDfs) btnDfs.addEventListener('click', () => runGraphTraversal('dfs'));

  // 9. Live Simulated Navigation & Route Exiting
  const btnLiveNav = document.getElementById('btn-live-nav');
  const btnStopSim = document.getElementById('btn-stop-sim');
  const btnExitRoute = document.getElementById('btn-exit-route');
  const btnClearActiveRoute = document.getElementById('btn-clear-active-route');
  const btnClearSource = document.getElementById('btn-clear-source');
  const btnClearDest = document.getElementById('btn-clear-dest');

  if (btnLiveNav) btnLiveNav.addEventListener('click', startSimulatedNavigation);
  if (btnStopSim) btnStopSim.addEventListener('click', () => exitNavigation(true));
  if (btnExitRoute) btnExitRoute.addEventListener('click', () => exitNavigation(true));
  if (btnClearActiveRoute) btnClearActiveRoute.addEventListener('click', () => exitNavigation(true));
  if (btnClearSource) btnClearSource.addEventListener('click', clearStartingLocation);
  if (btnClearDest) btnClearDest.addEventListener('click', clearDestination);

  // 10. Modals
  const btnAlgoLab = document.getElementById('btn-algo-lab');
  const btnDsaInsp = document.getElementById('btn-dsa-inspector');
  const btnViva = document.getElementById('btn-viva-guide');

  if (btnAlgoLab) btnAlgoLab.addEventListener('click', openAlgorithmLab);
  if (btnDsaInsp) btnDsaInsp.addEventListener('click', openDsaInspector);
  if (btnViva) btnViva.addEventListener('click', openVivaGuide);

  document.querySelectorAll('.btn-modal-close').forEach(btn => {
    btn.addEventListener('click', closeAllModals);
  });

  // 11. Viewport Toggles & 3D Earth Controls
  const btnToggle3D = document.getElementById('btn-toggle-3d');
  const btnOrbit = document.getElementById('btn-orbit-campus');
  const btnToggleBldg = document.getElementById('btn-toggle-buildings');
  const btnToggleLabels = document.getElementById('btn-toggle-labels');
  const btnToggleTraffic = document.getElementById('btn-toggle-traffic');
  const btnToggleGreen = document.getElementById('btn-toggle-greenery');

  if (btnToggle3D) btnToggle3D.addEventListener('click', toggle3DMode);
  if (btnOrbit) btnOrbit.addEventListener('click', toggleOrbitCampus);

  if (btnToggleBldg) {
    btnToggleBldg.addEventListener('click', () => {
      State.showBuildings = !State.showBuildings;
      btnToggleBldg.classList.toggle('active', State.showBuildings);
      renderBuildingsLayer();
    });
  }

  if (btnToggleLabels) {
    btnToggleLabels.addEventListener('click', () => {
      State.showLabels = !State.showLabels;
      btnToggleLabels.classList.toggle('active', State.showLabels);
      renderLabelsLayer();
    });
  }

  if (btnToggleTraffic) {
    btnToggleTraffic.addEventListener('click', () => {
      State.showTraffic = !State.showTraffic;
      btnToggleTraffic.classList.toggle('active', State.showTraffic);
      renderRoadsLayer();
    });
  }

  if (btnToggleGreen) {
    btnToggleGreen.addEventListener('click', () => {
      State.showGreenery = !State.showGreenery;
      btnToggleGreen.classList.toggle('active', State.showGreenery);
      renderGreeneryLayer();
    });
  }

  // 12. Zoom & Gizmo Controls
  const btnZoomIn = document.getElementById('btn-zoom-in');
  const btnZoomOut = document.getElementById('btn-zoom-out');
  const btnZoomReset = document.getElementById('btn-zoom-reset');
  const btnCompassNorth = document.getElementById('btn-compass-north');
  const btnTiltUp = document.getElementById('btn-tilt-up');
  const btnTiltDown = document.getElementById('btn-tilt-down');
  const btnRotateLeft = document.getElementById('btn-rotate-left');
  const btnRotateRight = document.getElementById('btn-rotate-right');

  if (btnZoomIn) btnZoomIn.addEventListener('click', zoomIn);
  if (btnZoomOut) btnZoomOut.addEventListener('click', zoomOut);
  if (btnZoomReset) btnZoomReset.addEventListener('click', resetZoom);
  if (btnCompassNorth) btnCompassNorth.addEventListener('click', resetNorth);
  if (btnTiltUp) btnTiltUp.addEventListener('click', tiltUp);
  if (btnTiltDown) btnTiltDown.addEventListener('click', tiltDown);
  if (btnRotateLeft) btnRotateLeft.addEventListener('click', rotateLeft);
  if (btnRotateRight) btnRotateRight.addEventListener('click', rotateRight);

  // 13. Mouse Drag Panning & Right-Click 3D Orbit / Tilt (Google Earth style)
  const mapContainer = document.getElementById('map-container');
  if (mapContainer) {
    mapContainer.addEventListener('contextmenu', (e) => e.preventDefault());

    mapContainer.addEventListener('mousedown', (e) => {
      if (e.target.closest('.svg-building-group') || e.target.closest('.svg-node-anchor')) return;
      if (State.isOrbiting) toggleOrbitCampus(); // Pause automatic orbit on user interaction

      if (e.button === 2 || e.shiftKey) {
        // 3D Tilt & Orbit Drag
        State.isRightDragging = true;
        State.dragStartX = e.clientX;
        State.dragStartY = e.clientY;
        State.startBearing = State.bearing;
        State.startPitch = State.pitch;
        mapContainer.style.cursor = 'grab';
      } else {
        // Standard Map Pan Drag
        State.isDragging = true;
        State.dragStartX = e.clientX - State.panX;
        State.dragStartY = e.clientY - State.panY;
        mapContainer.style.cursor = 'grabbing';
      }
    });

    window.addEventListener('mousemove', (e) => {
      if (State.isRightDragging) {
        const deltaX = e.clientX - State.dragStartX;
        const deltaY = e.clientY - State.dragStartY;

        State.is3D = true;
        const btn3d = document.getElementById('btn-toggle-3d');
        if (btn3d) btn3d.classList.add('active');

        // Horizontal drag rotates bearing; Vertical drag adjusts pitch
        State.bearing = (State.startBearing + (deltaX * 0.4)) % 360;
        State.pitch = Math.max(0, Math.min(65, State.startPitch - (deltaY * 0.3)));
        applySvgTransform();
      } else if (State.isDragging) {
        State.panX = e.clientX - State.dragStartX;
        State.panY = e.clientY - State.dragStartY;
        applySvgTransform();
      }
    });

    window.addEventListener('mouseup', () => {
      State.isDragging = false;
      State.isRightDragging = false;
      if (mapContainer) mapContainer.style.cursor = 'default';
    });

    mapContainer.addEventListener('wheel', (e) => {
      e.preventDefault();
      const delta = e.deltaY < 0 ? 0.08 : -0.08;
      State.zoomLevel = Math.max(0.6, Math.min(3.5, State.zoomLevel + delta));
      applySvgTransform();
    }, { passive: false });
  }
});
