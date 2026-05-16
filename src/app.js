const body = document.body;
const appMenu = document.querySelector('#app-menu');
const appSearch = document.querySelector('#app-search');
const runningApps = document.querySelector('#running-apps');
const quickSettings = document.querySelector('#quick-settings');
const clock = document.querySelector('#clock');
const terminalOutput = document.querySelector('#terminal-output');
const terminalForm = document.querySelector('#terminal-form');
const terminalInput = document.querySelector('#terminal-input');
const fileList = document.querySelector('#file-list');

const apps = {
  settings: { label: 'Ustawienia', windowId: 'settings-window' },
  files: { label: 'Menedżer plików', windowId: 'files-window' },
  terminal: { label: 'Terminal', windowId: 'terminal-window' },
};

function toggleTheme() {
  body.dataset.theme = body.dataset.theme === 'dark' ? 'light' : 'dark';
  updateRunningApps();
}

function openApp(appName) {
  const app = apps[appName];
  if (!app) return;

  document.querySelector(`#${app.windowId}`).classList.remove('hidden');
  appMenu.classList.add('hidden');
  quickSettings.classList.add('hidden');
  updateRunningApps();
}

function updateRunningApps() {
  const openApps = Object.values(apps)
    .filter((app) => !document.querySelector(`#${app.windowId}`).classList.contains('hidden'))
    .map((app) => app.label);

  runningApps.textContent = openApps.length
    ? `Aktywne: ${openApps.join(' · ')} | motyw: ${body.dataset.theme}`
    : `Brak aktywnych okien | motyw: ${body.dataset.theme}`;
}

function updateClock() {
  clock.textContent = new Intl.DateTimeFormat('pl-PL', {
    hour: '2-digit',
    minute: '2-digit',
  }).format(new Date());
}

function appendTerminalLine(text) {
  const line = document.createElement('p');
  line.textContent = text;
  terminalOutput.append(line);
  terminalOutput.scrollTop = terminalOutput.scrollHeight;
}

document.querySelector('#open-menu').addEventListener('click', () => {
  appMenu.classList.toggle('hidden');
  quickSettings.classList.add('hidden');
  appSearch.focus();
});

document.querySelector('#close-menu').addEventListener('click', () => {
  appMenu.classList.add('hidden');
});

document.querySelectorAll('[data-open-app]').forEach((button) => {
  button.addEventListener('click', () => openApp(button.dataset.openApp));
});

document.querySelectorAll('[data-close-window]').forEach((button) => {
  button.addEventListener('click', () => {
    document.querySelector(`#${button.dataset.closeWindow}`).classList.add('hidden');
    updateRunningApps();
  });
});

appSearch.addEventListener('input', () => {
  const query = appSearch.value.trim().toLowerCase();
  document.querySelectorAll('.app-tile').forEach((tile) => {
    tile.classList.toggle('hidden', !tile.dataset.name.includes(query));
  });
});

document.querySelector('#theme-toggle').addEventListener('click', toggleTheme);
document.querySelector('#quick-theme-toggle').addEventListener('click', toggleTheme);

document.querySelector('#status-toggle').addEventListener('click', () => {
  quickSettings.classList.toggle('hidden');
  appMenu.classList.add('hidden');
});

document.querySelectorAll('[data-accent]').forEach((button) => {
  button.addEventListener('click', () => {
    body.dataset.accent = button.dataset.accent;
    document.querySelectorAll('[data-accent]').forEach((dot) => dot.classList.remove('active'));
    button.classList.add('active');
  });
});

document.querySelector('#new-folder').addEventListener('click', () => {
  const folder = document.createElement('li');
  folder.innerHTML = '<span>📁</span> New Folder <small>folder</small>';
  fileList.prepend(folder);
});

document.querySelector('#refresh-files').addEventListener('click', () => {
  const marker = document.createElement('li');
  marker.innerHTML = '<span>↻</span> Odświeżono widok <small>teraz</small>';
  fileList.prepend(marker);
});

terminalForm.addEventListener('submit', (event) => {
  event.preventDefault();
  const command = terminalInput.value.trim();
  if (!command) return;

  if (command === 'clear') {
    terminalOutput.textContent = '';
  } else {
    appendTerminalLine(`myos@preview:/home$ ${command}`);
    const responses = {
      help: 'Dostępne komendy: help, date, clear, apps',
      date: new Date().toLocaleString('pl-PL'),
      apps: 'Aplikacje: Ustawienia, Menedżer plików, Terminal',
    };
    appendTerminalLine(responses[command] || `Nieznana komenda: ${command}`);
  }

  terminalInput.value = '';
});

updateClock();
updateRunningApps();
setInterval(updateClock, 30_000);
