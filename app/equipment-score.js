(function () {
  const categories = [
    { id: 'emblem', name: '문장', limit: 3, unit: '장' },
    { id: 'capsule', name: '캡슐', limit: 3, unit: '개' },
    { id: 'accessory', name: '악세', children: [
      { id: 'earring', name: '귀걸이', limit: 1, unit: '개' },
      { id: 'necklace', name: '목걸이', limit: 1, unit: '개' },
      { id: 'ring', name: '반지', limit: 1, unit: '개' }
    ] },
    { id: 'spirit', name: '스피릿', limit: 3, unit: '개' },
    { id: 'jewel', name: '보옥', limit: 1, unit: '개' },
    { id: 'darc', name: '디아크', limit: 1, unit: '개' }
  ];
  // Each catalog item will have id, name, score and one leaf category id.
  const items = [
  {
    "id": "emblem-courage",
    "name": "용기의 문장",
    "score": 100,
    "category": "emblem"
  },
  {
    "id": "emblem-sincerity",
    "name": "성실의 문장",
    "score": 100,
    "category": "emblem"
  },
  {
    "id": "emblem-shining-1",
    "name": "빛나는 알/로/오/듀 의 문장+1",
    "score": 110,
    "category": "emblem"
  },
  {
    "id": "emblem-shining-2",
    "name": "빛나는 알/로/오/듀 의 문장+2",
    "score": 120,
    "category": "emblem"
  },
  {
    "id": "emblem-shining-3",
    "name": "빛나는 알/로/오/듀 의 문장+3",
    "score": 130,
    "category": "emblem"
  },
  {
    "id": "emblem-shining-4",
    "name": "빛나는 알/로/오/듀 의 문장+4",
    "score": 140,
    "category": "emblem"
  },
  {
    "id": "emblem-shining-5",
    "name": "빛나는 알/로/오/듀 의 문장+5",
    "score": 150,
    "category": "emblem"
  },
  {
    "id": "emblem-shining-6",
    "name": "빛나는 알/로/오/듀 의 문장+6",
    "score": 160,
    "category": "emblem"
  },
  {
    "id": "emblem-shining-7",
    "name": "빛나는 알/로/오/듀 의 문장+7",
    "score": 170,
    "category": "emblem"
  },
  {
    "id": "emblem-shining-8",
    "name": "빛나는 알/로/오/듀 의 문장+8",
    "score": 180,
    "category": "emblem"
  },
  {
    "id": "emblem-shining-9",
    "name": "빛나는 알/로/오/듀 의 문장+9",
    "score": 190,
    "category": "emblem"
  },
  {
    "id": "emblem-shining-10",
    "name": "빛나는 알/로/오/듀 의 문장+10",
    "score": 200,
    "category": "emblem"
  },
  {
    "id": "emblem-magnamon-x",
    "name": "빛나는 매그너몬의 문장X",
    "score": 200,
    "category": "emblem"
  },
  {
    "id": "emblem-yggdrasil-d",
    "name": "이그드라실의 문장 D 라인",
    "score": 200,
    "category": "emblem",
    "maxCount": 3
  },
  {
    "id": "emblem-yggdrasil-c",
    "name": "이그드라실의 문장 C 라인",
    "score": 225,
    "category": "emblem",
    "maxCount": 3
  },
  {
    "id": "emblem-yggdrasil-b",
    "name": "이그드라실의 문장 B 라인",
    "score": 250,
    "category": "emblem",
    "maxCount": 3
  },
  {
    "id": "emblem-yggdrasil-a",
    "name": "이그드라실의 문장 A 라인",
    "score": 275,
    "category": "emblem",
    "maxCount": 3
  },
  {
    "id": "emblem-yggdrasil-s-plus",
    "name": "이그드라실의 문장 S+",
    "score": 300,
    "category": "emblem"
  },
  {
    "id": "capsule-red-0",
    "name": "레드 백신 캡슐",
    "score": 150,
    "category": "capsule",
    "maxCount": 3
  },
  {
    "id": "capsule-red-1",
    "name": "레드 데이터 캡슐",
    "score": 150,
    "category": "capsule",
    "maxCount": 3
  },
  {
    "id": "capsule-red-2",
    "name": "레드 바이러스 캡슐",
    "score": 150,
    "category": "capsule",
    "maxCount": 3
  },
  {
    "id": "capsule-shining-miracle",
    "name": "빛나는 기적의 캡슐",
    "score": 300,
    "category": "capsule",
    "maxCount": 1
  },
  {
    "id": "capsule-deep-sea",
    "name": "심해의 캡슐",
    "score": 300,
    "category": "capsule",
    "maxCount": 1
  },
  {
    "id": "capsule-pride",
    "name": "긍지의 캡슐",
    "score": 300,
    "category": "capsule",
    "maxCount": 1
  },
  {
    "id": "capsule-metal",
    "name": "메탈 캡슐",
    "score": 350,
    "category": "capsule",
    "maxCount": 1
  },
  {
    "id": "capsule-shining-metal",
    "name": "빛나는 메탈 캡슐",
    "score": 400,
    "category": "capsule",
    "maxCount": 1
  },
  {
    "id": "capsule-dark",
    "name": "암흑 캡슐",
    "score": 350,
    "category": "capsule",
    "maxCount": 1
  },
  {
    "id": "capsule-pitch-dark",
    "name": "칠흑빛 암흑 캡슐",
    "score": 400,
    "category": "capsule",
    "maxCount": 1
  },
  {
    "id": "capsule-abyss",
    "name": "심연의 캡슐",
    "score": 350,
    "category": "capsule",
    "maxCount": 1
  },
  {
    "id": "capsule-void-abyss",
    "name": "공허한 심연의 캡슐",
    "score": 400,
    "category": "capsule",
    "maxCount": 1
  },
  {
    "id": "accessory-subtle-earring",
    "name": "은은한 이그드라실 귀걸이",
    "score": 150,
    "category": "earring",
    "maxCount": 1
  },
  {
    "id": "accessory-subtle-necklace",
    "name": "은은한 이그드라실 목걸이",
    "score": 150,
    "category": "necklace",
    "maxCount": 1
  },
  {
    "id": "accessory-subtle-ring",
    "name": "은은한 이그드라실 반지",
    "score": 150,
    "category": "ring",
    "maxCount": 1
  },
  {
    "id": "accessory-fancy-earring",
    "name": "화려한 이그드라실 귀걸이",
    "score": 200,
    "category": "earring",
    "maxCount": 1
  },
  {
    "id": "accessory-fancy-necklace",
    "name": "화려한 이그드라실 목걸이",
    "score": 200,
    "category": "necklace",
    "maxCount": 1
  },
  {
    "id": "accessory-fancy-ring",
    "name": "화려한 이그드라실 반지",
    "score": 200,
    "category": "ring",
    "maxCount": 1
  },
  {
    "id": "accessory-obsidian-earring",
    "name": "옵시디언디지조이드 귀걸이",
    "score": 400,
    "category": "earring",
    "maxCount": 1
  },
  {
    "id": "accessory-choi-necklace",
    "name": "최영재의 목걸이",
    "score": 400,
    "category": "necklace",
    "maxCount": 1
  },
  {
    "id": "accessory-nibelungen-ring",
    "name": "니벨룽겐의 반지",
    "score": 400,
    "category": "ring",
    "maxCount": 1
  },
  {
    "id": "accessory-spider-earring",
    "name": "거미 여왕의 귀걸이",
    "score": 450,
    "category": "earring",
    "maxCount": 1
  },
  {
    "id": "accessory-gold-general-necklace",
    "name": "황금 장군의 목걸이",
    "score": 450,
    "category": "necklace",
    "maxCount": 1
  },
  {
    "id": "accessory-forest-devil-ring",
    "name": "깊은 숲 악마의 반지",
    "score": 450,
    "category": "ring",
    "maxCount": 1
  },
  {
    "id": "accessory-shining-spider-earring",
    "name": "빛나는 거미 여왕의 귀걸이",
    "score": 500,
    "category": "earring",
    "maxCount": 1
  },
  {
    "id": "accessory-shining-gold-general-necklace",
    "name": "빛나는 황금 장군의 목걸이",
    "score": 500,
    "category": "necklace",
    "maxCount": 1
  },
  {
    "id": "accessory-shining-forest-devil-ring",
    "name": "빛나는 깊은 숲 악마의 반지",
    "score": 500,
    "category": "ring",
    "maxCount": 1
  },
  {
    "id": "spirit-incomplete",
    "name": "불완전한 스피릿",
    "score": 100,
    "category": "spirit",
    "maxCount": 3
  },
  {
    "id": "spirit-complete",
    "name": "완전한 스피릿",
    "score": 200,
    "category": "spirit",
    "maxCount": 3
  },
  {
    "id": "spirit-double-lowest",
    "name": "최하급 더블 스피릿",
    "score": 200,
    "category": "spirit",
    "maxCount": 3
  },
  {
    "id": "spirit-double-low",
    "name": "하급 더블 스피릿",
    "score": 225,
    "category": "spirit",
    "maxCount": 3
  },
  {
    "id": "spirit-double-middle",
    "name": "중급 더블 스피릿",
    "score": 250,
    "category": "spirit",
    "maxCount": 3
  },
  {
    "id": "spirit-double-high",
    "name": "상급 더블 스피릿",
    "score": 270,
    "category": "spirit",
    "maxCount": 3
  },
  {
    "id": "spirit-double-highest",
    "name": "최상급 더블 스피릿",
    "score": 300,
    "category": "spirit",
    "maxCount": 3
  },
  {
    "id": "jewel-dongle",
    "name": "동글몬 보옥",
    "score": 200,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-beast-attendant",
    "name": "성수보좌 보옥",
    "score": 50,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-sacred-beast-1",
    "name": "성수 보옥+1",
    "score": 50,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-sacred-beast-2",
    "name": "성수 보옥+2",
    "score": 100,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-sacred-beast-3",
    "name": "성수 보옥+3",
    "score": 150,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-sacred-beast-4",
    "name": "성수 보옥+4",
    "score": 200,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-sacred-beast-5",
    "name": "성수 보옥+5",
    "score": 250,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-sacred-beast-6",
    "name": "성수 보옥+6",
    "score": 300,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-sacred-beast-7",
    "name": "성수 보옥+7",
    "score": 350,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-sacred-beast-8",
    "name": "성수 보옥+8",
    "score": 400,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-sacred-beast-9",
    "name": "성수 보옥+9",
    "score": 450,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "jewel-sacred-beast-10",
    "name": "성수 보옥+10",
    "score": 500,
    "category": "jewel",
    "maxCount": 1
  },
  {
    "id": "darc-health",
    "name": "체력디아크",
    "score": 100,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-defense",
    "name": "방어디아크",
    "score": 100,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-low",
    "name": "하급 마왕의 디아크",
    "score": 150,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-middle",
    "name": "중급 마왕의 디아크",
    "score": 200,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-high",
    "name": "상급 마왕의 디아크",
    "score": 300,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-3-d",
    "name": "3단 마왕의 디아크 D 라인",
    "score": 200,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-3-c",
    "name": "3단 마왕의 디아크 C 라인",
    "score": 250,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-3-b",
    "name": "3단 마왕의 디아크 B 라인",
    "score": 300,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-3-a",
    "name": "3단 마왕의 디아크 A 라인",
    "score": 350,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-3-s",
    "name": "3단 마왕의 디아크 S 라인",
    "score": 400,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-4-d",
    "name": "4단 마왕의 디아크 D 라인",
    "score": 300,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-4-c",
    "name": "4단 마왕의 디아크 C 라인",
    "score": 350,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-4-b",
    "name": "4단 마왕의 디아크 B 라인",
    "score": 400,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-4-a",
    "name": "4단 마왕의 디아크 A 라인",
    "score": 450,
    "category": "darc",
    "maxCount": 1
  },
  {
    "id": "darc-demon-4-s",
    "name": "4단 마왕의 디아크 S 라인",
    "score": 500,
    "category": "darc",
    "maxCount": 1
  }
];
  const groups = categories.flatMap(category => category.children || [category]);
  const storageKey = 'dg-equipment-score-selected-v1';
  let saved = [];
  try {
    const value = JSON.parse(localStorage.getItem(storageKey));
    if (Array.isArray(value)) saved = value;
  } catch {}
  const selected = [];
  saved.forEach(id => {
    const item = items.find(item => item.id === id);
    const group = item && groups.find(group => group.id === item.category);
    if (group && selected.filter(id => items.find(item => item.id === id)?.category === group.id).length < group.limit && selected.filter(selectedId => selectedId === id).length < (item.maxCount ?? group.limit)) selected.push(id);
  });

  window.renderEquipmentScore = function () {
    const panel = document.querySelector('#reviewPanel');
    panel.innerHTML = '<h2>장비 점수</h2>' +
      '<div class="equipment-score-total"><span>총 장비 점수</span><output id="equipmentTotal" aria-live="polite">0</output></div>' +
      '<div class="equipment-score-layout"><section class="equipment-score-equipped" aria-label="장착 장비"><h3>장착 장비</h3><div id="equipmentSlots" class="equipment-score-slots"></div><button type="button" id="equipmentReset" class="equipment-score-reset" aria-label="장착 장비 초기화">초기화</button></section>' +
      '<section class="equipment-score-catalog"><h3 class="equipment-score-items-title">아이템 목록</h3><div id="equipmentItems"></div></section></div>';
    const list = panel.querySelector('#equipmentItems');
    const slotGrid = panel.querySelector('#equipmentSlots');
    const slots = [];
    const numerals = ['Ⅰ', 'Ⅱ', 'Ⅲ'];
    groups.forEach(group => {
      for (let index = 0; index < group.limit; index++) {
        const box = document.createElement('div'); box.className = 'equipment-score-slot';
        const caption = group.name + (group.limit > 1 ? ' ' + numerals[index] : '');
        box.setAttribute('aria-label', caption);
        const title = document.createElement('span'); title.className = 'equipment-score-slot-label'; title.textContent = caption;
        const name = document.createElement('strong'); name.className = 'equipment-score-slot-name';
        const score = document.createElement('span'); score.className = 'equipment-score-slot-points';
        box.append(title, name, score); slotGrid.append(box); slots.push({ group, index, box, name, score, caption });
      }
    });
    const controls = [];
    const badges = [];
    const countSelected = group => selected.filter(id => items.find(item => item.id === id)?.category === group.id).length;
    const itemCount = id => selected.filter(selectedId => selectedId === id).length;
    panel.querySelector('#equipmentReset').onclick = () => { selected.length = 0; updateTotal(); };
    function setQuantity(item, group, requested) {
      const current = itemCount(item.id);
      const maximum = Math.min(item.maxCount ?? group.limit, group.limit - countSelected(group) + current);
      const target = Math.max(0, Math.min(maximum, Math.trunc(Number(requested) || 0)));
      for (let count = current; count > target; count--) selected.splice(selected.lastIndexOf(item.id), 1);
      for (let count = current; count < target; count++) selected.push(item.id);
      updateTotal();
    }
    function updateTotal() {
      const total = selected.reduce((sum, id) => sum + (items.find(item => item.id === id)?.score || 0), 0);
      panel.querySelector('#equipmentTotal').textContent = total.toLocaleString('ko-KR', { maximumFractionDigits: 6 });
      slots.forEach(slot => {
        const equipped = [...selected].map(id => items.find(item => item.id === id)).filter(item => item?.category === slot.group.id);
        const item = equipped[slot.index];
        slot.name.textContent = item ? item.name.replace('빛나는 알/로/오/듀 의 문장+', '빛나는 문장 +').replace('이그드라실의 문장 S+', '이그드라실 S+').replace('빛나는 매그너몬의 문장X', '매그너몬 문장X') : '';
        slot.score.textContent = item ? item.score.toLocaleString('ko-KR') + '점' : '';
        slot.box.classList.toggle('filled', Boolean(item));
        slot.box.setAttribute('aria-label', slot.caption + (item ? ': ' + item.name + ', ' + item.score + '점' : ': 빈 칸'));
        slot.box.title = slot.caption + (item ? '\n' + item.name + '\n' + item.score + '점' : '\n빈 칸');
      });
      badges.forEach(({ group, badge }) => { badge.textContent = countSelected(group) + ' / ' + group.limit + group.unit; });
      controls.forEach(({ group, check, item, label, quantity }) => {
        const count = itemCount(item.id);
        check.checked = count > 0;
        check.disabled = !check.checked && countSelected(group) >= group.limit;
        label.classList.toggle('equipment-score-disabled', check.disabled);
        if (quantity) {
          quantity.value = String(count || 1);
          quantity.disabled = count === 0;
          const available = Math.min(item.maxCount ?? group.limit, group.limit - countSelected(group) + count);
          [...quantity.options].forEach(option => { option.disabled = Number(option.value) > available; });
        }
      });
      try { localStorage.setItem(storageKey, JSON.stringify([...selected])); } catch {}
    }
    function renderGroup(group, parent, nested) {
      const block = document.createElement('section'); block.className = 'equipment-score-group';
      block.setAttribute('aria-label', group.name);
      const head = document.createElement('div'); head.className = 'equipment-score-category-head';
      const title = document.createElement(nested ? 'h4' : 'h3'); title.textContent = group.name;
      const badge = document.createElement('span'); badge.className = 'equipment-score-capacity'; badge.setAttribute('aria-live', 'polite');
      badges.push({ group, badge }); head.append(title, badge); block.append(head);
      const options = document.createElement('div'); options.className = 'traitoptions';
      addDisclosure(group, head, options);
      const groupItems = items.filter(item => item.category === group.id);
      groupItems.forEach(item => {
        const label = document.createElement('div'); label.className = 'traitoption equipment-score-item';
        const choice = document.createElement('label'); choice.className = 'equipment-score-choice';
        const check = document.createElement('input'); check.type = 'checkbox'; check.checked = itemCount(item.id) > 0;
        check.setAttribute('aria-label', item.name);
        check.onchange = () => setQuantity(item, group, check.checked ? 1 : 0);
        const name = document.createElement('span'); name.textContent = item.name;
        const score = document.createElement('strong'); score.textContent = '+' + item.score.toLocaleString('ko-KR') + '점';
        choice.append(check, name); label.append(choice, score);
        let quantity = null;
        const itemLimit = Math.min(item.maxCount ?? group.limit, group.limit);
        if (itemLimit > 1) {
          quantity = document.createElement('select'); quantity.setAttribute('aria-label', item.name + ' 수량');
          for (let count = 1; count <= itemLimit; count++) {
            const option = document.createElement('option'); option.value = String(count); option.textContent = count + group.unit; quantity.append(option);
          }
          quantity.onchange = () => setQuantity(item, group, quantity.value);
          label.append(quantity);
        } else {
          const limit = document.createElement('span'); limit.className = 'equipment-score-item-limit'; limit.textContent = '최대 1' + group.unit;
          label.append(limit);
        }
        options.append(label); controls.push({ group, item, check, label, quantity });
      });
      if (!groupItems.length) {
        const note = document.createElement('p'); note.className = 'wbnote'; note.textContent = '아이템 등록 전'; options.append(note);
      }
      block.append(options); parent.append(block);
    }
    function addDisclosure(group, head, content) {
      content.id = 'equipment-options-' + group.id;
      content.hidden = true;
      const toggle = document.createElement('button'); toggle.type = 'button'; toggle.className = 'equipment-score-toggle';
      toggle.textContent = '+'; toggle.setAttribute('aria-expanded', 'false'); toggle.setAttribute('aria-controls', content.id);
      toggle.setAttribute('aria-label', group.name + ' 목록 펼치기');
      toggle.onclick = () => {
        content.hidden = !content.hidden;
        toggle.textContent = content.hidden ? '+' : '−';
        toggle.setAttribute('aria-expanded', String(!content.hidden));
        toggle.setAttribute('aria-label', group.name + (content.hidden ? ' 목록 펼치기' : ' 목록 접기'));
      };
      head.append(toggle);
    }
    categories.forEach(category => {
      const section = document.createElement('section'); section.className = 'equipment-score-category';
      if (category.children) {
        const head = document.createElement('div'); head.className = 'equipment-score-category-head';
        const title = document.createElement('h3'); title.textContent = category.name; head.append(title); section.append(head);
        const children = document.createElement('div'); children.className = 'equipment-score-accessories';
        addDisclosure(category, head, children);
        category.children.forEach(group => renderGroup(group, children, true)); section.append(children);
      } else renderGroup(category, section, false);
      list.append(section);
    });
    updateTotal();
  };
})();
