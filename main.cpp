#include <bits/stdc++.h>
#include <ncurses.h>

using namespace std;

#define printd(expr) std::cout << #expr " = " << (expr) << std::endl
#define locale static

using ubyte = unsigned char;

namespace colors {
  struct rgb_format {
    ubyte red{};
    ubyte green{};
    ubyte blue{};
  };
}

struct vec2i {
  int x{};
  int y{};
};

namespace typoi {
  template <int width, int height>
  class ppm_image {
  private:
    vector<vector<colors::rgb_format>> pixels{height, vector<colors::rgb_format>(width)};
  public:
   ppm_image() = default;
   void set_pixel(int xpos, int ypos, colors::rgb_format color) {
     assert(xpos >= 0 and xpos < width);
     assert(ypos >= 0 and ypos < height);
     pixels[ypos][xpos] = color;
   }
   void flush(const string &path) {
     stringstream stream;
     stream << "P3" << endl;
     stream << width << " " << height << endl;
     stream << "255" << endl;
     for (int i = 0; i < height; i++) {
       for (int j = 0; j < width; j++) {
         stream << uint(pixels[i][j].red)   << " ";
         stream << uint(pixels[i][j].green) << " ";
         stream << uint(pixels[i][j].blue)  << endl;
       }
     }
     ofstream fout(path);
     assert(fout.is_open());
     fout << stream.str();
     assert(fout.good());
     fout.close();
   }
   void backround(const colors::rgb_format &color) {
     for (int i = 0; i < height; i++) {
       for (int j = 0; j < width; j++) {
         pixels[i][j] = color;
       }
     }
   }
   void line(vec2i a, vec2i b, const colors::rgb_format &color) {
     bool is_swap = false;
     if (abs(a.x - b.x) < abs(a.y - b.y)) {
       swap(a.x, a.y);
       swap(b.x, b.y);
       is_swap = true;
     }
     if (a.x > b.x)
       swap(a, b);
    int delta_x = abs(a.x - b.x);
    int delta_y = abs(a.y - b.y);
    int error = 0;
    int delta_err = delta_y;
    int y = a.y;
    int diry = b.y - a.y;
    if (diry > 0)
      diry = 1;
    if (diry < 0)
      diry = -1;
    for (int x = a.x; x <= b.x; x++) {
      if (is_swap) {
        this->set_pixel(y, x, color);
      } else
        this->set_pixel(x, y, color);
      error += delta_err;
      if ((error << 1) >= delta_x) {
        y += diry;
        error -= delta_x;
      }
    }
   }
   void rectangle(vec2i a, vec2i b, vec2i c, vec2i d, const colors::rgb_format &color) {
     this->line(a, b, color);
     this->line(b, c, color);
     this->line(c, d, color);
     this->line(d, a, color);
   }
   void flip_vertical() {
     reverse(pixels.begin(), pixels.end());
   }
   void flip_horizontal() {
     for (auto &i : pixels)
       reverse(i.begin(), i.end());
   }
  private:
   ppm_image(const ppm_image &) = delete;
   void operator=(const ppm_image &) = delete;
  };

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
        after.push(ch);
        n_items++;
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
      for (int i = 0; i < after.n_items; i++)
        file << after[i];
      for (int i = before.n_items - 1; i >= 0; i--)
        file << before[i];
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
  };
}

int main() {

  initscr();
  noecho();
  raw();
  keypad(stdscr, true);
  start_color();

  init_pair(1, COLOR_YELLOW, COLOR_BLACK); // numbers
  init_pair(2, COLOR_RED, COLOR_BLACK); // delims
  init_pair(3, COLOR_WHITE, COLOR_BLACK); // other
  init_pair(4, COLOR_GREEN, COLOR_BLACK); // keyword
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // keyword

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

  typoi::gap_buffer text("out.cpp");

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
    case 19: {
      curs_set(0);
      auto win = newwin(3, 9, (rows - 3) / 2, (cols - 9) / 2);
      box(win, 0, 0);
      wmove(win, 1, 1);
      wprintw(win, "save it");
      wrefresh(win);
      text.write("out.cpp");
      this_thread::sleep_for(chrono::milliseconds(500));
      curs_set(1);
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
      if (cx < cols - 1 and cx < lines.get_line_len(cy + ldx))
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
    case 127:
      if (text.n_items > 0 and (cy > 0 || cx > 0)) {
//        char ech = text[lines.get_line_start(cy + ldx) + sdx + cx];
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
    if (text.changed() || ldx != prev_ldx || sdx != prev_sdx) {
      lines.update(text);
      highlight.reserve(text.n_items);
      int i = 0;
      while (i < text.n_items) {
        if (i < text.n_items and isdigit(text[i]) and (i > 0 || !isalpha(text[i - 1])))
          highlight[i++] = 1;
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
        } else if (i < text.n_items and strchr("+-*/^%!()[]{}<>&=", text[i]))
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

  text.write("out.cpp");

  return 0;
}
