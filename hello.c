int a = 1;
int b = 2;

void f() {
  a = b;
}

int main() {
  f();
  return a;
}