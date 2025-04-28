#include "kernel/types.h"
#include "user/user.h"

char*  get_month_name(int month) {
  static const char* months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  return (month >= 1 && month <= 12) ? (char*)months[month-1] : "Unknown";
}

void pad_zero2(char* buf, int num) {
  if(num < 10) {
    buf[0] = '0';
    buf[1] = '0' + num;
  } else {
    buf[0] = '0' + (num / 10);
    buf[1] = '0' + (num % 10);
  }
  buf[2] = '\0';
}

int check_leap(int y) {
  return (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
}

int month_days(int m, int y) {
  static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  return (m == 2 && check_leap(y)) ? 29 : days[m-1];
}

void convert_time(uint64 sec, int* y, int* m, int* d, int* h, int* min, int* s) {
  *h = (sec / 3600) % 24;
  *min = (sec / 60) % 60;
  *s = sec % 60;
  
  int days = sec / 86400;
  *y = 1970;
  
  while(1) {
    int days_in_year = check_leap(*y) ? 366 : 365;
    if(days >= days_in_year) {
      days -= days_in_year;
      (*y)++;
    } else break;
  }
  
  *m = 1;
  while(*m <= 12) {
    int dim = month_days(*m, *y);
    if(days >= dim) {
      days -= dim;
      (*m)++;
    } else break;
  }
  *d = days + 1;
}

int main() {
  uint64 nsec = gettime();
  int y, m, d, h, min, s;
  char h_str[3], min_str[3], s_str[3];
  
  convert_time(nsec / 1000000000, &y, &m, &d, &h, &min, &s);
  
  pad_zero2(h_str, h);
  pad_zero2(min_str, min);
  pad_zero2(s_str, s);

  printf("%d-%s-%d %s:%s:%s.%lu\n",
         y, get_month_name(m), d, h_str, min_str, s_str, nsec % 1000000000);
  
  exit(0);
}
