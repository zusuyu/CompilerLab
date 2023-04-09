void f(int arr[]) {
  arr[1] = arr[0] + 2;
}

int main() {
  int a[3][2] = {1, 2, 3, 4, 5, 6};
  f(a[0]);
  f(a[1]);
  f(a[2]);
  return 0;
}