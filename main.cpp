#include <bits/stdc++.h>
#include <ncurses.h>

using namespace std;

#define printd(expr) std::cout << #expr " = " << (expr) << std::endl
#define locale static

locale const int tab_width = 2;

namespace typoi {
  struct stack {
    vector<char> items{};
    int n_items{};
    void push(char item) {
      items.push_back(item);
      n_items++;
    }
    char pop() {
      if (n_items > 0) {
        char res_value = items.back();
        items.pop_back();
        n_items--;
        return res_value;
      }
      throw range_error("stack::pop() -> stack is empty");
    }
    char operator[](int idx) const {
      if (idx < 0 || idx >= n_items)
        throw range_error("stack::operator[](int) -> out of range");
      return items[idx];
    }
  };

  struct gap_buffer {
    stack after{};
    stack before{};
    int n_items{};
    bool change{};
    gap_buffer() = default;
    gap_buffer(string path) {
      ifstream stream(path);
      assert(stream.is_open());
      char ch;
      while (stream.get(ch)) {
        if (ch == '\t') {
          for (int i = 0; i < tab_width; i++) {
            after.push(' ');
            n_items++;
          }
        } else {
          after.push(ch);
          n_items++;
        }
      }
      assert(stream.eof());
      stream.close();
      change = true;
    }
    void load(string path) {
      ifstream stream(path);
      if (!stream.is_open())
        return;
      char ch;
      while (stream.get(ch)) {
        if (ch == '\t') {
          for (int i = 0; i < tab_width; i++) {
            after.push(' ');
            n_items++;
          }
        } else {
          after.push(ch);
          n_items++;
        }
      }
      assert(stream.eof());
      stream.close();
      change = true;
    }
    void insert(char ch, int offset) {
      if (offset < 0 || offset > n_items)
        throw invalid_argument("invalid offset");
      if (offset != after.n_items)
        move(offset);
      after.push(ch);
      n_items++;
      change = true;
    }
    void erase_prev(int offset) {
      assert(n_items > 0);
      if (offset < 1 || offset > n_items)
        throw invalid_argument("invalid offset");
      if (offset != after.n_items)
        move(offset);
      after.pop();
      n_items--;
      change = true;
    }
    void erase_curr(int offset) {
      assert(n_items > 0);
      if (offset < 0 || offset >= n_items)
        throw invalid_argument("invalid offset");
      if (offset + 1 != after.n_items)
        move(offset + 1);
      after.pop();
      n_items--;
      change = true;
    }
    char operator[](int idx) {
      if (idx < after.n_items)
        return after[idx];
      return before[before.n_items - 1 - (idx - after.n_items)];
    }
    void show() {
      for (int i = 0; i < after.n_items; i++)
        cout << after[i];
      for (int i = before.n_items - 1; i >= 0; i--)
        cout << before[i];
      cout << endl;
    }
    void write(string path) {
      ofstream file(path);
      assert(file.is_open());
      char last{};
      for (int i = 0; i < after.n_items; i++) {
        last = after[i];
        file << last;
      }
      for (int i = before.n_items - 1; i >= 0; i--) {
        last = before[i];
        file << last;
      }
      if (last != '\n')
        file << '\n';
      assert(file.good());
      file.close();
    }
    bool changed() {
      return change;
    }
    void no_changed() {
      change = false;
    }
  private:
    void move(int offset) {
      if (offset < after.n_items) {
        int dis = after.n_items - offset;
        for (int i = 0; i < dis; i++)
          before.push(after.pop());
      } else {
        int dis = offset - after.n_items;
        for (int i = 0; i < dis; i++)
          after.push(before.pop());
      }
    }
  };

  struct line_set {
    struct line_set_info {
      int start{};
      int end{};
      int length{};
    };
    vector<line_set_info> lines_info{};
    int n_lines{};
    void update(const gap_buffer &text) {
      lines_info.clear();
      n_lines = 0;
      int start = 0;
      int end = 0;
      for (int i = 0; i < text.after.n_items; i++, end++) {
        if (text.after[i] == '\n') {
          lines_info.push_back({start, end, end - start + 1});
          start = end + 1;
          n_lines++;
        }
      }
      for (int i = text.before.n_items - 1; i >= 0; i--, end++) {
        if (text.before[i] == '\n') {
          lines_info.push_back({start, end, end - start + 1});
          start = end + 1;
          n_lines++;
        }
      }
      if (start < end) {
        lines_info.push_back({start, end - 1, end - start});
        n_lines++;
      }
    }
    int get_line_len(int n) {
      if (n_lines == 0)
        return 0;
      return lines_info[n].length;
    }
    int get_line_start(int n) {
      if (n_lines == 0)
        return 0;
      if (n >= n_lines) {
        int start = 0;
        for (int i = 0; i < n_lines; i++)
          start += lines_info[i].length;
        return start;
      }
      return lines_info[n].start;
    }
    int get_line_end(int n) {
      if (n_lines == 0)
        return 0;
      return lines_info[n].end;
    }
  };
}

#define ctrl_key(code) ((code) & 0x1F)

int main(int argc, char *argv[]) {
  if (argc == 1) {
    cerr << "usage: bad [file]" << endl;
    return 1;
  }
  setlocale(LC_ALL, "Russian");
  initscr();
  noecho();
  raw();
  keypad(stdscr, true);
  start_color();

  init_color(COLOR_BLUE, 200, 500, 1000);
  init_color(COLOR_YELLOW, 1000, 1000, 0);
  init_color(COLOR_GREEN, 0, 1000, 0);
  init_color(COLOR_RED, 1000, 0, 0);
  init_color(COLOR_MAGENTA, 300, 300, 300);

  init_pair(1, COLOR_BLUE, COLOR_BLACK); // numbers
  init_pair(2, COLOR_MAGENTA, COLOR_BLACK); // delims
  init_pair(3, COLOR_WHITE, COLOR_BLACK); // other
  init_pair(4, COLOR_YELLOW, COLOR_BLACK); // keyword
  init_pair(5, COLOR_BLUE, COLOR_BLACK); // preprocessor

  int rows{};
  int cols{};

  getmaxyx(stdscr, rows, cols);

  int cx{};
  int cy{};
  int ch{};

  int ldx{};
  int sdx{};

  int prev_ldx{};
  int prev_sdx{};
  bool save_it{};

  typoi::gap_buffer text;

  if (argc > 1)
    text.load(argv[1]);

  typoi::line_set lines;
  lines.update(text);

  vector<int> highlight{};
  const unordered_map<string, bool> key_words {
    {"alignas", true},
    {"alignof", true},
    {"and", true},
    {"and_eq", true},
    {"asm", true},
    {"atomic_cancel", true},
    {"atomic_commit", true},
    {"atomic_noexcept", true},
    {"auto", true},
    {"bitand", true},
    {"bitor", true},
    {"bool", true},
    {"break", true},
    {"case", true},
    {"catch", true},
    {"char", true},
    {"char16_t", true},
    {"char32_t", true},
    {"char8_t", true},
    {"class", true},
    {"co_await", true},
    {"compl", true},
    {"concept", true},
    {"const", true},
    {"const_cast", true},
    {"consteval", true},
    {"constexpr", true},
    {"constinit", true},
    {"continue", true},
    {"co_return", true},
    {"co_yield", true},
    {"decltype", true},
    {"default", true},
    {"delete", true},
    {"do", true},
    {"double", true},
    {"dynamic_cast", true},
    {"else", true},
    {"enum", true},
    {"explicit", true},
    {"export", true},
    {"extern", true},
    {"false", true},
    {"float", true},
    {"for", true},
    {"friend", true},
    {"goto", true},
    {"if", true},
    {"inline", true},
    {"int", true},
    {"long", true},
    {"mutable", true},
    {"namespace", true},
    {"new", true},
    {"noexcept", true},
    {"not", true},
    {"not_eq", true},
    {"nullptr", true},
    {"operator", true},
    {"or", true},
    {"or_eq", true},
    {"private", true},
    {"protected", true},
    {"public", true},
    {"reflexpr", true},
    {"register", true},
    {"reinterpret_cast", true},
    {"requires", true},
    {"return", true},
    {"short", true},
    {"signed", true},
    {"sizeof", true},
    {"static", true},
    {"static_assert", true},
    {"static_cast", true},
    {"struct", true},
    {"switch", true},
    {"synchronized", true},
    {"template", true},
    {"this", true},
    {"thread_local", true},
    {"throw", true},
    {"true", true},
    {"try", true},
    {"typedef", true},
    {"typeid", true},
    {"typename", true},
    {"union", true},
    {"unsigned", true},
    {"using", true},
    {"virtual", true},
    {"void", true},
    {"volatile", true},
    {"wchar_t", true},
    {"while", true},
    {"xor", true},
    {"xor_eq", true},
    {"override", true},
    {"elif", true},
    {"func", true},
    {"method", true},
    {"none", true},
    {"null", true},
    {"cmake_minimum_required", true},
    {"project", true},
    {"add_executable", true},
    {"target_link_libraries", true},
    {"band", true}
  };

  const unordered_map<string, bool> preprocess_key_word {
    {"define", true},
    {"elif", true},
    {"else", true},
    {"endif", true},
    {"error", true},
    {"if", true},
    {"ifdef", true},
    {"ifndef", true},
    {"include", true},
    {"line", true},
    {"pragma", true},
    {"undef", true}
  };

  do {
    switch (ch) {
    case ctrl_key('s'): {
      curs_set(0);
      int n = strlen(argv[1]);
      auto win = newwin(3, n + 2, (rows - 3) / 2, (cols - n - 2) / 2);
      box(win, 0, 0);
      wmove(win, 1, 1);
      wprintw(win, "%s", argv[1]);
      wrefresh(win);
      text.write(argv[1]);
      this_thread::sleep_for(chrono::milliseconds(500));
      curs_set(1);
      save_it = true;
    }
    break;
    case ctrl_key('k'): {
      if (text.n_items > 0) {
        for (int i = lines.get_line_end(cy + ldx) + 1; i > lines.get_line_start(cy + ldx); i--)
          text.erase_prev(i);
        cx = 0;
        sdx = 0;
      }
    }
    break;
    case KEY_HOME:
      cx = 0;
      sdx = 0;
      break;
    case KEY_END:
      if (cols > lines.get_line_len(cy + ldx))
        cx = lines.get_line_len(cy + ldx) - 1;
      else {
        cx = cols - 1;
        sdx = lines.get_line_len(cy + ldx) - cols;
      }
      break;
    case KEY_LEFT:
      if (cx > 0)
        cx--;
      else if (sdx > 0)
        sdx--;
      else if (cy > 0) {
        cy--;
        if (cols > lines.get_line_len(cy + ldx))
          cx = lines.get_line_len(cy + ldx) - 1;
        else {
          cx = cols - 1;
          sdx = lines.get_line_len(cy + ldx) - cols;
        }
      } else if (ldx > 0) {
        if (cols > lines.get_line_len(cy + ldx))
          cx = lines.get_line_len(cy + ldx) - 1;
        else {
          cx = cols - 1;
          sdx = lines.get_line_len(cy + ldx) - cols;
        }
      }
      break;
    case KEY_RIGHT:
      if (lines.n_lines == 0)
        break;
      if (cx < cols - 1 and cx < lines.get_line_len(cy + ldx) and text[lines.get_line_start(cy + ldx) + sdx + cx] != '\n')
        cx++;
      else if (cx == cols - 1 and lines.get_line_len(cy + ldx) > sdx + cols)
        sdx++;
      else if (ldx + cy < lines.n_lines - 1) {
        if (cy < rows - 1)
          cy++;
        else
          ldx++;
        cx = 0;
        sdx = 0;
      }
      break;
    case KEY_UP: {
      if (cy > 0)
        cy--;
      else if (ldx > 0)
        ldx--;
      if (cx < lines.get_line_len(cy + ldx))
        break;
      if (cx >= lines.get_line_len(cy + ldx)) {
        cx = lines.get_line_len(cy + ldx) - 1;
        sdx = 0;
      }
      else if (lines.get_line_len(cy + ldx) > cols) {
        cx = cols - 1;
        sdx = lines.get_line_len(cy + ldx) - cols;
        if (sdx < 0)
          sdx = 0;
      }
    }
      break;
    case KEY_DOWN: {
      if (cy < rows - 1 and cy + ldx < lines.n_lines - 1)
        cy++;
      else if (cy == rows - 1 and lines.n_lines > ldx + rows)
        ldx++;
      if (cx < lines.get_line_len(cy + ldx))
        break;
      if (cx >= lines.get_line_len(cy + ldx)) {
        cx = lines.get_line_len(cy + ldx) - 1;
        sdx = 0;
      }
      else if (lines.get_line_len(cy + ldx) > cols) {
        cx = cols - 1;
        sdx = lines.get_line_len(cy + ldx) - cols;
        if (sdx < 0)
          sdx = 0;
      }
    }
      break;
    case KEY_BACKSPACE:
      if (text.n_items > 0 and lines.get_line_start(cy + ldx) + sdx + cx > 0) {
        text.erase_prev(lines.get_line_start(cy + ldx) + sdx + cx);
        if (cx > 0)
          cx--;
        else if (sdx > 0)
          sdx--;
        else if (cy > 0) {
          cy--;
          cx = lines.get_line_len(cy + ldx) - 1;
        }
        else if (ldx > 0) {
          ldx--;
          cx = lines.get_line_len(cy + ldx) - 1;
        }
      }
      break;
    case '\t':
    {
      for (int i = 0; i < tab_width; i++) {
        text.insert(' ', lines.get_line_start(cy + ldx) + sdx + cx);
        if (cx < cols - 1)
          cx++;
        else
          sdx++;
      }
    }
      break;
    default:
      if (isprint(ch) || isspace(ch)) {
        text.insert(char(ch), lines.get_line_start(cy + ldx) + sdx + cx);
        if (cx < cols - 1)
          cx++;
        else
          sdx++;
        if (ch == '\n') {
          if (cy < rows - 1)
            cy++;
          else
            ldx++;
          cx = 0;
          sdx = 0;
        }
      }
    }
    if (text.changed() || ldx != prev_ldx || sdx != prev_sdx || save_it) {
      save_it = false;
      lines.update(text);
      highlight.reserve(text.n_items);
      int i = 0;
      while (i < text.n_items) {
        if (isdigit(text[i]) || text[i] == '.') {
          if (isdigit(text[i]))
            highlight[i++] = 1;
          if (i < text.n_items) {
//            0xF12323ULL
//            0b11010111
            if (tolower(text[i]) == 'x') {
              highlight[i++] = 1;
              while (i < text.n_items and (isdigit(text[i]) || strchr("abcdef", tolower(text[i]))))
                highlight[i++] = 1;
            } else if (tolower(text[i]) == 'b') {
              highlight[i++] = 1;
              while (i < text.n_items and strchr("01", text[i]))
                highlight[i++] = 1;
            } else if (isdigit(text[i])) {
              while (i < text.n_items and isdigit(text[i]))
                highlight[i++] = 1;
              if (i < text.n_items) {
                if (text[i] == '.') {
                  highlight[i++] = 1;
                  while (i < text.n_items and isdigit(text[i]))
                    highlight[i++] = 1;
                }
              }
            } else if (text[i] == '.') {
              if (i + 1 < text.n_items and isdigit(text[i + 1])) {
                highlight[i++] = 1;
                while (i < text.n_items and isdigit(text[i]))
                  highlight[i++] = 1;
              }
              else
                highlight[i++] = 3;
            }
            if (i < text.n_items and tolower(text[i]) == 'u')
              highlight[i++] = 1;
            if (i < text.n_items and tolower(text[i]) == 'l')
              highlight[i++] = 1;
            if (i < text.n_items and tolower(text[i]) == 'l')
              highlight[i++] = 1;
            if (i < text.n_items and tolower(text[i]) == 'f')
              highlight[i++] = 1;
          }
        }
        else if (isalpha(text[i]) || text[i] == '_') {
          int beg = i;
          string word{};
          while (i < text.n_items and (isalnum(text[i]) || text[i] == '_'))
            word += text[i++];
          if (key_words.find(word) != key_words.end()) {
            while (beg < i)
              highlight[beg++] = 4;
          } else if (preprocess_key_word.find(word) != preprocess_key_word.end()) {
            while (beg < i)
              highlight[beg++] = 5;
          } else {
            while (beg < i)
              highlight[beg++] = 3;
          }
        } else if (i < text.n_items and strchr("+-*/^%!$()[]{}<>&=|_#\'\"", text[i]))
          highlight[i++] = 2;
        else if (i < text.n_items)
          highlight[i++] = 3;
      }
      clear();
      move(0, 0);
      for (int i = 0; i < rows and i + ldx < lines.n_lines; i++) {
        bool nnl = true;
        for (int j = 0; j < cols - 1 and j + sdx < lines.get_line_len(i + ldx); j++) {
          char pch = text[lines.get_line_start(i + ldx) + sdx + j];
          if (pch == '\n') {
            if (i == rows - 1)
              continue;
            nnl = false;
          }
          attron(COLOR_PAIR(highlight[lines.get_line_start(i + ldx) + sdx + j]));
          if (pch == ' ')
            addch('.');
          else
            addch(pch);
        }
        if (nnl)
          addch('\n');
      }
      refresh();
      text.no_changed();
      prev_ldx = ldx;
      prev_sdx = sdx;
    }
    move(cy, cx);
  } while ((ch = getch()) != 17);

  endwin();

  return 0;
}
