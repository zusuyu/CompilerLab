void f(int arr[], int idx) {
  arr[idx] = 114;
}
int main() {
  int a[3];
  f(a, 1);
  putint(a[1]);
  return 233;
}