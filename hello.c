/* block comment in a line */

int main() {
  // 忽略我的存在
  /* ??? */

  int a = 1;
  int b = 2;
  {
    int a = 3;
    b = a * a;
  }
  {
    int c = 4;
    return a + b + c;

  }

  /* **** */
}

/*

  block
  comment
  in
  many
  lines

*/