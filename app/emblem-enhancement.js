function enhancementIcon(label, icon, directory = 'emblem-enhancement') {
  const content = document.createElement('span');
  content.className = 'emblem-icon-label';
  const image = document.createElement('img');
  image.src = `assets/${directory}/${icon}.png`;
  image.alt = '';
  content.append(image, label);
  return content;
}

function addEnhancementTable(panel, title, headers, rows, notes = []) {
  const section = document.createElement('section');
  section.className = 'emblem-enhancement';
  const heading = document.createElement('h2');
  heading.id = 'enhancement-table-' + panel.childElementCount;
  heading.textContent = title;
  section.append(heading);
  for (const note of notes) {
    const line = document.createElement('p');
    line.className = 'wbnote';
    line.textContent = note;
    section.append(line);
  }
  const wrap = document.createElement('div');
  wrap.className = 'wbtablewrap';
  wrap.tabIndex = 0;
  const table = document.createElement('table');
  table.className = 'wbtable';
  table.setAttribute('aria-labelledby', heading.id);
  const header = table.createTHead().insertRow();
  for (const label of headers) {
    const cell = document.createElement('th');
    cell.scope = 'col';
    cell.append(label);
    header.append(cell);
  }
  const body = table.createTBody();
  for (const [stage, ...values] of rows) {
    const row = body.insertRow();
    const label = document.createElement('th');
    label.scope = 'row';
    label.textContent = stage;
    row.append(label);
    for (const value of values) row.insertCell().append(value);
  }
  wrap.append(table);
  section.append(wrap);
  panel.append(section);
}

function renderEmblemEnhancement() {
  const emblems = [
    ['오메가몬', 'omegamon', [
      ['공격력', [130, 130, 135, 135, 145, 145, 155, 155, 165, 185]],
      ['HP', [300, 310, 320, 330, 340, 360, 380, 400, 420, 460]],
      ['전투등급', [400, 410, 420, 430, 440, 460, 480, 500, 520, 560]]
    ]],
    ['로드나이트몬', 'lordknightmon', [
      ['공격력', [100, 105, 110, 115, 125, 135, 145, 155, 165, 185]],
      ['전투등급', [500, 535, 570, 605, 675, 745, 815, 885, 955, 1095]]
    ]],
    ['듀나스몬', 'dynasmon', [
      ['방어력', [80, 80, 85, 85, 90, 95, 100, 105, 110, 120]],
      ['HP', [400, 435, 470, 505, 575, 645, 715, 785, 855, 995]]
    ]],
    ['알파몬', 'alphamon', [
      ['공격력', [150, 150, 155, 160, 170, 170, 180, 180, 190, 210]],
      ['VP', [400, 415, 430, 445, 475, 505, 535, 565, 595, 655]]
    ]]
  ];
  const red = ['발바몬의 정수', 'material-red'];
  const green = ['데크스몬의 정수', 'material-green'];
  const white = ['리바이어의 정수', 'material-white'];
  const materials = [
    [2, red, 1, 90, 1, 90],
    [3, red, 2, 80, 4, 100],
    [4, green, 2, 70, 6, 100],
    [5, green, 3, 60, 12, 100],
    [6, white, 4, 45, 40, 100],
    [7, white, 4, 35, 80, 100],
    [8, red, 5, 25, 150, 100],
    [9, green, 8, 15, 400, 100],
    [10, white, 10, 10, 900, 100]
  ];

  const panel = document.querySelector('#reviewPanel');
  panel.replaceChildren();

  const statRows = Array.from({ length: 10 }, (_, index) => [
    '+' + (index + 1),
    ...emblems.map(([, , stats]) => stats.map(([label, values]) => `${label} +${values[index].toLocaleString('ko-KR')}`).join('\n'))
  ]);
  const statHeaders = ['강화 단계', ...emblems.map(([name, icon]) => enhancementIcon(name + ' 문장', icon))];
  addEnhancementTable(panel, '단계별 문장 능력치', statHeaders, statRows);

  const materialRows = materials.map(([stage, material, normalCount, normalChance, guaranteedCount, guaranteedChance]) => {
    const commonCount = (stage - 1) * 100;
    return [
      '+' + stage,
      enhancementIcon(...material),
      `${normalCount}개 · ${normalChance}%`,
      `${guaranteedCount}개 · ${guaranteedChance}%`,
      enhancementIcon(commonCount + '개', 'material-ring'),
      enhancementIcon(commonCount + '개', 'material-gold')
    ];
  });
  addEnhancementTable(panel, '강화 재료와 성공 확률', [
    '목표 단계', '단계별 재료', '일반 강화 수량', '확정 강화 수량', '아이템 부스터', '디지몬 축복'
  ], materialRows, [
    '일반 강화는 1억 Bit가 필요하며, 실패하면 강화 단계가 하락합니다.',
    '확정 강화는 2억 Bit가 필요합니다. +1에서 +2로 올릴 때는 확정 강화가 적용되지 않습니다.',
    '재료는 첨부 자료에 나온 아이콘 모양으로 표시했습니다.'
  ]);
}

function renderJewelEnhancement() {
  const panel = document.querySelector('#reviewPanel');
  panel.replaceChildren();

  const jewels = [
    ['현무몬의 보옥', 'xuanwumon', '건강', '단일'],
    ['백호몬의 보옥', 'baihumon', '민첩', '전체'],
    ['주작몬의 보옥', 'zhuqiaomon', '힘', '원거리']
  ];
  const statHeaders = [
    '강화 단계',
    ...jewels.map(([name, icon]) => enhancementIcon(name, icon, 'jewel-enhancement'))
  ];
  const statRows = Array.from({ length: 10 }, (_, index) => {
    const stage = index + 1;
    return [
      '+' + stage,
      ...jewels.map(([, , stat, skill]) => `${stat} +${stage * 10}\n${skill} 스킬 데미지 +${stage}%`)
    ];
  });
  addEnhancementTable(panel, '단계별 보옥 능력치', statHeaders, statRows);

  const orb = ['징표', [535, 536, 537]];
  const symbol = ['힘의정수', [540, 541, 542]];
  const leaf = ['힘의근원', [545, 546, 547]];
  const materials = [
    [2, orb, 1, '—'],
    [3, orb, 2, '4개'],
    [4, symbol, 4, '7개'],
    [5, symbol, 5, '12개'],
    [6, leaf, 9, '40개'],
    [7, leaf, 10, '80개'],
    [8, orb, 4, '150개'],
    [9, symbol, 7, '400개'],
    [10, leaf, 12, '900개']
  ];

  const rows = materials.map(([stage, [label, iconIds], normal, guaranteed]) => {
    const material = document.createElement('span');
    material.className = 'emblem-icon-label';
    for (const id of iconIds) {
      const icon = document.createElement('img');
      icon.src = `assets/jewel-enhancement/material-${id}.png`;
      icon.alt = '';
      material.append(icon);
    }
    material.append(label);
    const commonCount = (stage - 1) * 100;
    return [
      '+' + stage,
      material,
      normal + '개',
      guaranteed,
      enhancementIcon(commonCount + '개', 'material-ring'),
      enhancementIcon(commonCount + '개', 'material-gold')
    ];
  });
  addEnhancementTable(panel, '단계별 보옥 강화 재료', [
    '목표 단계', '단계별 재료', '일반 강화 수량', '확정 강화 수량', '아이템 부스터', '디지몬 축복'
  ], rows, [
    '일반 강화는 2억 Bit, 확정 강화는 4억 Bit가 필요합니다.',
    '+2는 일반 강화만 가능합니다. 아이템 부스터와 디지몬 축복은 두 강화 방식에 같은 수량이 필요합니다.',
    '단계별 재료 아이콘은 왼쪽부터 현무몬, 백호몬, 주작몬 순서입니다.'
  ]);
}
