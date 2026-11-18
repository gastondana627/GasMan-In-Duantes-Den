/*
 * malbolge.c - Ben Olmstead's 1998 Malbolge Specification Interpreter
 * Compile in VS Code terminal: cc -O2 malbolge.c -o malbolge
 * Run single-file 10-frame animation: ./malbolge avalanche_all_10_frames.mal
 * Run high-res ANSI color frames:
 *   for f in frame_*.mal; do ./malbolge "$f"; sleep 0.25; done
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

const char *xlat1 =
  "+b(29e*j1VMEKLyC})8&m#~W>qxdRp0wkrUo[D7,XTcA\"lI.v%{gJh4G\\-=O@5`_3i<?Z';FNQuY]szf$!BS/|t:Pn6^Ha";
const char *xlat2 =
  "5z]&gqtyfr$(we4{WP)H-Zn,[%\\3dL+Q;>U!pJS72FhOA1CB6v^=I_0/8|jsb9m<.TVac`uY*MK'X~xDl}REokN:#?G\"i@";

unsigned short crz(unsigned short a, unsigned short b) {
  unsigned short r = 0, p = 1;
  static const unsigned short table[3][3] = {{1,0,0},{1,0,2},{2,2,1}};
  for (int i = 0; i < 10; i++) {
    r += table[a % 3][b % 3] * p;
    a /= 3; b /= 3; p *= 3;
  }
  return r;
}

int main(int argc, char **argv) {
  if (setvbuf(stdout, NULL, _IOLBF, 0) != 0) {
    perror("setvbuf");
    return 1;
  }
  if (argc < 2) { fprintf(stderr, "Usage: %s <file.mal>\n", argv[0]); return 1; }
  FILE *f = fopen(argv[1], "r");
  if (!f) { perror("fopen"); return 1; }
  unsigned short *mem = malloc(sizeof(unsigned short) * 59049);
  if (!mem) {
    perror("malloc");
    fclose(f);
    return 1;
  }
  int i = 0, x;
  while ((x = getc(f)) != EOF) {
    if (isspace(x)) continue;
    if (i == 59049) {
      fprintf(stderr, "Program > 59049 words\n");
      fclose(f);
      free(mem);
      return 1;
    }
    if (x < 33 || x > 126) {
      fprintf(stderr, "Invalid character\n");
      fclose(f);
      free(mem);
      return 1;
    }
    if (!strchr("ji*p</vo", xlat1[(x - 33 + i) % 94])) {
      fprintf(stderr, "Invalid opcode at %d\n", i);
      fclose(f);
      free(mem);
      return 1;
    }
    mem[i++] = x;
  }
  if (ferror(f)) {
    perror("read");
    fclose(f);
    free(mem);
    return 1;
  }
  fclose(f);
  if (i < 2) {
    fprintf(stderr, "Program must contain at least two words\n");
    free(mem);
    return 1;
  }
  while (i < 59049) { mem[i] = crz(mem[i - 1], mem[i - 2]); i++; }
  unsigned short a = 0, c = 0, d = 0;
  int home_escape_state = 0;
  for (;;) {
    if (mem[c] < 33 || mem[c] > 126) break;
    switch (xlat1[(mem[c] - 33 + c) % 94]) {
      case 'j': d = mem[d]; break;
      case 'i': c = mem[d]; break;
      case '*': a = mem[d] = (mem[d] / 3) + (mem[d] % 3) * 19683; break;
      case 'p': a = mem[d] = crz(a, mem[d]); break;
      case '<': {
        int output = a & 0xFF;
        putchar(output);
        if (output == '\x1b') {
          home_escape_state = 1;
        } else if (home_escape_state == 1) {
          home_escape_state = (output == '[') ? 2 : 0;
        } else if (home_escape_state == 2) {
          home_escape_state = 0;
          if (output == 'H') {
            fflush(stdout);
            usleep(200000);
          }
        }
        break;
      }
      case '/': { int ch = getchar(); a = (ch == EOF) ? 59048 : ch; break; }
      case 'v': free(mem); return 0;
    }
    if (mem[c] >= 33 && mem[c] <= 126) mem[c] = xlat2[mem[c] - 33];
    c = (c == 59048) ? 0 : c + 1;
    d = (d == 59048) ? 0 : d + 1;
  }
  free(mem);
  return 0;
}
