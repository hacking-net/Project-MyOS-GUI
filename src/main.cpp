#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <unistd.h>

namespace {

struct Color {
  unsigned long pixel{};
};

struct Rect {
  int x{};
  int y{};
  int w{};
  int h{};

  bool contains(const int px, const int py) const {
    return px >= x && px <= x + w && py >= y && py <= y + h;
  }
};

struct Palette {
  Color background;
  Color surface;
  Color surfaceStrong;
  Color text;
  Color muted;
  Color border;
  Color accent;
  Color terminal;
  Color terminalText;
};

enum class Theme { Dark, Light };
enum class Accent { Sky, Violet, Amber };
enum class AppId { Settings, Files, Terminal };

enum class HitAction {
  Noop,
  ToggleMenu,
  ToggleQuickSettings,
  ToggleTheme,
  OpenSettings,
  OpenFiles,
  OpenTerminal,
  CloseSettings,
  CloseFiles,
  CloseTerminal,
  AccentSky,
  AccentViolet,
  AccentAmber,
  NewFolder,
  RefreshFiles,
  FocusSearch,
  FocusTerminal,
};

struct HitRegion {
  Rect rect;
  HitAction action{HitAction::Noop};
};

std::string lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

std::string nowTime() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t current = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
  localtime_r(&current, &tm);
  char buffer[16]{};
  std::strftime(buffer, sizeof(buffer), "%H:%M", &tm);
  return buffer;
}

std::string nowDateTime() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t current = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
  localtime_r(&current, &tm);
  char buffer[64]{};
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
  return buffer;
}

class MyOSGui {
 public:
  MyOSGui() {
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
      std::fprintf(stderr, "Nie można otworzyć serwera X11. Uruchom program w sesji graficznej.\n");
      std::exit(1);
    }

    screen_ = DefaultScreen(display_);
    window_ = XCreateSimpleWindow(
      display_, RootWindow(display_, screen_), 80, 80, width_, height_, 0,
      BlackPixel(display_, screen_), BlackPixel(display_, screen_));

    XStoreName(display_, window_, "MyOS GUI Prototype — C++");
    XSelectInput(display_, window_, ExposureMask | ButtonPressMask | KeyPressMask | StructureNotifyMask);

    wmDelete_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display_, window_, &wmDelete_, 1);

    gc_ = XCreateGC(display_, window_, 0, nullptr);
    font_ = XLoadQueryFont(display_, "fixed");
    if (font_) {
      XSetFont(display_, gc_, font_->fid);
    }

    XMapWindow(display_, window_);
    loadPalette();
    terminalLines_ = {
      "myos@preview:/home$ help",
      "Dostepne komendy: help, date, clear, apps"
    };
  }

  ~MyOSGui() {
    if (font_) XFreeFont(display_, font_);
    if (gc_) XFreeGC(display_, gc_);
    if (window_) XDestroyWindow(display_, window_);
    if (display_) XCloseDisplay(display_);
  }

  void run() {
    while (running_) {
      while (XPending(display_) > 0) {
        XEvent event{};
        XNextEvent(display_, &event);
        handleEvent(event);
      }

      const auto currentClock = nowTime();
      if (currentClock != clock_) {
        clock_ = currentClock;
        draw();
      }

      XFlush(display_);
      usleep(16000);
    }
  }

 private:
  Display* display_{};
  int screen_{};
  Window window_{};
  GC gc_{};
  XFontStruct* font_{};
  Atom wmDelete_{};
  bool running_{true};
  int width_{1180};
  int height_{740};
  Theme theme_{Theme::Dark};
  Accent accent_{Accent::Sky};
  Palette palette_{};
  bool menuOpen_{false};
  bool quickSettingsOpen_{false};
  bool settingsOpen_{false};
  bool filesOpen_{false};
  bool terminalOpen_{false};
  bool searchFocused_{false};
  bool terminalFocused_{false};
  std::string searchQuery_;
  std::string terminalInput_;
  std::string clock_{nowTime()};
  std::vector<std::string> terminalLines_;
  std::vector<std::string> files_{"[DIR] Documents", "[DIR] Pictures", "[TXT] readme.txt", "[CFG] myos.config"};
  std::vector<HitRegion> hits_;

  Color color(const char* value) const {
    XColor parsed{};
    XColor exact{};
    const Colormap colormap = DefaultColormap(display_, screen_);
    if (!XAllocNamedColor(display_, colormap, value, &parsed, &exact)) {
      return {BlackPixel(display_, screen_)};
    }
    return {parsed.pixel};
  }

  void loadPalette() {
    const bool dark = theme_ == Theme::Dark;
    palette_.background = color(dark ? "#0f172a" : "#f8fafc");
    palette_.surface = color(dark ? "#111827" : "#ffffff");
    palette_.surfaceStrong = color(dark ? "#1f2937" : "#e2e8f0");
    palette_.text = color(dark ? "#e5e7eb" : "#111827");
    palette_.muted = color(dark ? "#94a3b8" : "#64748b");
    palette_.border = color(dark ? "#334155" : "#cbd5e1");
    palette_.terminal = color("#020617");
    palette_.terminalText = color("#d1fae5");

    switch (accent_) {
      case Accent::Sky: palette_.accent = color("#38bdf8"); break;
      case Accent::Violet: palette_.accent = color("#a78bfa"); break;
      case Accent::Amber: palette_.accent = color("#f59e0b"); break;
    }
  }

  void handleEvent(XEvent& event) {
    if (event.type == ClientMessage && static_cast<Atom>(event.xclient.data.l[0]) == wmDelete_) {
      running_ = false;
      return;
    }

    if (event.type == ConfigureNotify) {
      width_ = event.xconfigure.width;
      height_ = event.xconfigure.height;
      draw();
      return;
    }

    if (event.type == Expose) {
      draw();
      return;
    }

    if (event.type == ButtonPress) {
      handleClick(event.xbutton.x, event.xbutton.y);
      return;
    }

    if (event.type == KeyPress) {
      handleKey(event.xkey);
    }
  }

  void handleClick(const int x, const int y) {
    searchFocused_ = false;
    terminalFocused_ = false;

    for (auto it = hits_.rbegin(); it != hits_.rend(); ++it) {
      if (it->rect.contains(x, y)) {
        dispatch(it->action);
        draw();
        return;
      }
    }

    if (menuOpen_ && !Rect{28, height_ - 430, 420, 340}.contains(x, y)) {
      menuOpen_ = false;
    }
    if (quickSettingsOpen_ && !Rect{width_ - 320, height_ - 265, 280, 175}.contains(x, y)) {
      quickSettingsOpen_ = false;
    }
    draw();
  }

  void handleKey(XKeyEvent& keyEvent) {
    KeySym key{};
    char buffer[32]{};
    const int length = XLookupString(&keyEvent, buffer, sizeof(buffer), &key, nullptr);

    if (key == XK_Escape) {
      menuOpen_ = false;
      quickSettingsOpen_ = false;
      searchFocused_ = false;
      terminalFocused_ = false;
      draw();
      return;
    }

    std::string* target = nullptr;
    if (searchFocused_) target = &searchQuery_;
    if (terminalFocused_) target = &terminalInput_;

    if (!target) return;

    if (key == XK_BackSpace) {
      if (!target->empty()) target->pop_back();
    } else if (key == XK_Return && terminalFocused_) {
      submitTerminalCommand();
    } else if (length > 0 && std::isprint(static_cast<unsigned char>(buffer[0]))) {
      target->push_back(buffer[0]);
    }

    draw();
  }

  void dispatch(const HitAction action) {
    switch (action) {
      case HitAction::ToggleMenu:
        menuOpen_ = !menuOpen_;
        quickSettingsOpen_ = false;
        searchFocused_ = menuOpen_;
        break;
      case HitAction::ToggleQuickSettings:
        quickSettingsOpen_ = !quickSettingsOpen_;
        menuOpen_ = false;
        break;
      case HitAction::ToggleTheme:
        theme_ = theme_ == Theme::Dark ? Theme::Light : Theme::Dark;
        loadPalette();
        break;
      case HitAction::OpenSettings: openApp(AppId::Settings); break;
      case HitAction::OpenFiles: openApp(AppId::Files); break;
      case HitAction::OpenTerminal: openApp(AppId::Terminal); break;
      case HitAction::CloseSettings: settingsOpen_ = false; break;
      case HitAction::CloseFiles: filesOpen_ = false; break;
      case HitAction::CloseTerminal: terminalOpen_ = false; break;
      case HitAction::AccentSky: accent_ = Accent::Sky; loadPalette(); break;
      case HitAction::AccentViolet: accent_ = Accent::Violet; loadPalette(); break;
      case HitAction::AccentAmber: accent_ = Accent::Amber; loadPalette(); break;
      case HitAction::NewFolder: files_.insert(files_.begin(), "[DIR] New Folder"); break;
      case HitAction::RefreshFiles: files_.insert(files_.begin(), "[OK ] Widok odswiezony"); break;
      case HitAction::FocusSearch: searchFocused_ = true; break;
      case HitAction::FocusTerminal: terminalFocused_ = true; break;
      case HitAction::Noop: break;
    }
  }

  void openApp(const AppId app) {
    menuOpen_ = false;
    quickSettingsOpen_ = false;
    searchFocused_ = false;
    switch (app) {
      case AppId::Settings: settingsOpen_ = true; break;
      case AppId::Files: filesOpen_ = true; break;
      case AppId::Terminal:
        terminalOpen_ = true;
        terminalFocused_ = true;
        break;
    }
  }

  void submitTerminalCommand() {
    const std::string command = lower(terminalInput_);
    if (command.empty()) return;

    if (command == "clear") {
      terminalLines_.clear();
    } else {
      terminalLines_.push_back("myos@preview:/home$ " + terminalInput_);
      if (command == "help") {
        terminalLines_.push_back("Dostepne komendy: help, date, clear, apps");
      } else if (command == "date") {
        terminalLines_.push_back(nowDateTime());
      } else if (command == "apps") {
        terminalLines_.push_back("Aplikacje: Ustawienia, Menedzer plikow, Terminal");
      } else {
        terminalLines_.push_back("Nieznana komenda: " + terminalInput_);
      }
    }

    terminalInput_.clear();
  }

  void draw() {
    hits_.clear();
    fill({0, 0, width_, height_}, palette_.background);
    drawDesktop();
    if (settingsOpen_) drawSettingsWindow();
    if (filesOpen_) drawFilesWindow();
    if (terminalOpen_) drawTerminalWindow();
    if (menuOpen_) drawAppMenu();
    if (quickSettingsOpen_) drawQuickSettings();
    drawStatusPanel();
  }

  void fill(const Rect rect, const Color color) const {
    XSetForeground(display_, gc_, color.pixel);
    XFillRectangle(display_, window_, gc_, rect.x, rect.y, rect.w, rect.h);
  }

  void outline(const Rect rect, const Color color) const {
    XSetForeground(display_, gc_, color.pixel);
    XDrawRectangle(display_, window_, gc_, rect.x, rect.y, rect.w, rect.h);
  }

  void text(const int x, const int y, const std::string& value, const Color color) const {
    XSetForeground(display_, gc_, color.pixel);
    XDrawString(display_, window_, gc_, x, y, value.c_str(), static_cast<int>(value.size()));
  }

  void card(const Rect rect) const {
    fill(rect, palette_.surface);
    outline(rect, palette_.border);
  }

  void button(const Rect rect, const std::string& label, const HitAction action, const bool primary = false) {
    fill(rect, primary ? palette_.accent : palette_.surfaceStrong);
    outline(rect, primary ? palette_.accent : palette_.border);
    text(rect.x + 12, rect.y + 22, label, primary ? palette_.background : palette_.text);
    hits_.push_back({rect, action});
  }

  void drawDesktop() {
    text(34, 46, "MyOS Preview", palette_.accent);
    text(34, 72, "Minimalny prototyp GUI napisany w C++ i X11", palette_.text);

    button({34, 112, 118, 72}, "[F] Pliki", HitAction::OpenFiles);
    button({34, 198, 118, 72}, "[>] Terminal", HitAction::OpenTerminal);
    button({34, 284, 118, 72}, "[*] Ustawienia", HitAction::OpenSettings);

    const Rect welcome{width_ - 430, 36, 380, 130};
    if (welcome.x > 190) {
      card(welcome);
      text(welcome.x + 18, welcome.y + 32, "Pulpit MyOS", palette_.accent);
      text(welcome.x + 18, welcome.y + 62, "Otworz menu, aplikacje lub szybkie ustawienia.", palette_.text);
      text(welcome.x + 18, welcome.y + 92, "Escape zamyka panele pomocnicze.", palette_.muted);
    }
  }

  void drawAppMenu() {
    const Rect menu{28, height_ - 430, 420, 340};
    card(menu);
    text(menu.x + 18, menu.y + 32, "Menu aplikacji", palette_.text);
    button({menu.x + 356, menu.y + 12, 42, 32}, "X", HitAction::ToggleMenu);

    const Rect search{menu.x + 18, menu.y + 62, 384, 38};
    fill(search, palette_.surfaceStrong);
    outline(search, searchFocused_ ? palette_.accent : palette_.border);
    text(search.x + 12, search.y + 24, searchQuery_.empty() ? "Szukaj aplikacji..." : searchQuery_, searchQuery_.empty() ? palette_.muted : palette_.text);
    hits_.push_back({search, HitAction::FocusSearch});

    struct Tile { std::string label; std::string tags; HitAction action; };
    const std::array<Tile, 3> tiles{{
      {"[*] Ustawienia", "ustawienia motyw wyglad", HitAction::OpenSettings},
      {"[F] Menedzer plikow", "pliki menedzer folder", HitAction::OpenFiles},
      {"[>] Terminal", "terminal konsola komendy", HitAction::OpenTerminal},
    }};

    int y = menu.y + 118;
    const std::string query = lower(searchQuery_);
    for (const auto& tile : tiles) {
      if (!query.empty() && lower(tile.tags + " " + tile.label).find(query) == std::string::npos) continue;
      button({menu.x + 18, y, 384, 54}, tile.label, tile.action);
      y += 66;
    }
  }

  void drawWindowFrame(const Rect rect, const std::string& title, const HitAction closeAction) {
    card(rect);
    fill({rect.x, rect.y, rect.w, 42}, palette_.surfaceStrong);
    outline({rect.x, rect.y, rect.w, 42}, palette_.border);
    text(rect.x + 14, rect.y + 27, title, palette_.text);
    button({rect.x + rect.w - 44, rect.y + 8, 28, 26}, "X", closeAction);
  }

  void drawSettingsWindow() {
    const Rect win = centeredWindow(760, 350, 90);
    drawWindowFrame(win, "Ustawienia", HitAction::CloseSettings);
    text(win.x + 24, win.y + 78, "Motyw", palette_.accent);
    text(win.x + 24, win.y + 106, theme_ == Theme::Dark ? "Aktywny: ciemny" : "Aktywny: jasny", palette_.text);
    button({win.x + 24, win.y + 126, 170, 38}, "Przelacz motyw", HitAction::ToggleTheme, true);

    text(win.x + 250, win.y + 78, "Akcent", palette_.accent);
    button({win.x + 250, win.y + 96, 90, 38}, "Sky", HitAction::AccentSky, accent_ == Accent::Sky);
    button({win.x + 350, win.y + 96, 100, 38}, "Violet", HitAction::AccentViolet, accent_ == Accent::Violet);
    button({win.x + 460, win.y + 96, 100, 38}, "Amber", HitAction::AccentAmber, accent_ == Accent::Amber);

    text(win.x + 24, win.y + 210, "System", palette_.accent);
    text(win.x + 24, win.y + 238, "MyOS GUI Prototype C++ / build 0.2.0", palette_.text);
    text(win.x + 24, win.y + 268, "Opcje demo: animacje wlaczone, uklad standardowy", palette_.muted);
  }

  void drawFilesWindow() {
    const Rect win = centeredWindow(820, 430, 120);
    drawWindowFrame(win, "Menedzer plikow", HitAction::CloseFiles);
    const Rect sidebar{win.x + 18, win.y + 62, 160, win.h - 82};
    fill(sidebar, palette_.surfaceStrong);
    outline(sidebar, palette_.border);
    text(sidebar.x + 14, sidebar.y + 30, "Pulpit", palette_.text);
    text(sidebar.x + 14, sidebar.y + 62, "Dokumenty", palette_.text);
    text(sidebar.x + 14, sidebar.y + 94, "Pobrane", palette_.text);

    const int mainX = win.x + 198;
    text(mainX, win.y + 84, "/home/myos", palette_.muted);
    button({mainX, win.y + 102, 130, 36}, "Nowy folder", HitAction::NewFolder);
    button({mainX + 144, win.y + 102, 105, 36}, "Odswiez", HitAction::RefreshFiles);

    int y = win.y + 160;
    for (const auto& file : files_) {
      if (y > win.y + win.h - 24) break;
      fill({mainX, y - 20, win.w - 230, 30}, palette_.surfaceStrong);
      outline({mainX, y - 20, win.w - 230, 30}, palette_.border);
      text(mainX + 12, y, file, palette_.text);
      y += 38;
    }
  }

  void drawTerminalWindow() {
    const Rect win = centeredWindow(820, 420, 150);
    drawWindowFrame(win, "Terminal", HitAction::CloseTerminal);
    const Rect body{win.x + 18, win.y + 58, win.w - 36, win.h - 76};
    fill(body, palette_.terminal);
    outline(body, palette_.border);

    int y = body.y + 26;
    const int first = std::max(0, static_cast<int>(terminalLines_.size()) - 10);
    for (int i = first; i < static_cast<int>(terminalLines_.size()); ++i) {
      text(body.x + 14, y, terminalLines_[i], palette_.terminalText);
      y += 24;
    }

    const Rect input{body.x + 12, body.y + body.h - 42, body.w - 24, 30};
    outline(input, terminalFocused_ ? palette_.accent : palette_.border);
    text(input.x + 10, input.y + 20, "myos@preview:/home$ " + terminalInput_, palette_.terminalText);
    hits_.push_back({input, HitAction::FocusTerminal});
  }

  void drawQuickSettings() {
    const Rect panel{width_ - 320, height_ - 265, 280, 175};
    card(panel);
    text(panel.x + 18, panel.y + 32, "Szybkie ustawienia", palette_.text);
    button({panel.x + 18, panel.y + 58, 244, 38}, "Przelacz motyw", HitAction::ToggleTheme, true);
    button({panel.x + 18, panel.y + 106, 244, 38}, "Pelne ustawienia", HitAction::OpenSettings);
  }

  void drawStatusPanel() {
    const Rect panel{20, height_ - 70, width_ - 40, 50};
    card(panel);
    button({panel.x + 12, panel.y + 9, 110, 32}, "Menu", HitAction::ToggleMenu, true);

    std::string running = "Brak aktywnych okien";
    std::vector<std::string> open;
    if (settingsOpen_) open.push_back("Ustawienia");
    if (filesOpen_) open.push_back("Pliki");
    if (terminalOpen_) open.push_back("Terminal");
    if (!open.empty()) {
      running = "Aktywne: ";
      for (std::size_t i = 0; i < open.size(); ++i) {
        if (i > 0) running += " | ";
        running += open[i];
      }
    }
    text(panel.x + 150, panel.y + 31, running, palette_.muted);

    const Rect status{panel.x + panel.w - 250, panel.y + 9, 232, 32};
    button(status, "Wi-Fi  87%  " + clock_, HitAction::ToggleQuickSettings);
  }

  Rect centeredWindow(int desiredWidth, int desiredHeight, int yOffset) const {
    const int w = std::min(desiredWidth, width_ - 70);
    const int h = std::min(desiredHeight, height_ - 130);
    return {std::max(20, (width_ - w) / 2), std::max(30, yOffset), w, h};
  }
};

}  // namespace

int main() {
  MyOSGui app;
  app.run();
  return 0;
}
