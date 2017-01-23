#ifndef gmt_H
#define gmt_H
class gmt
{
  public:
  bool sync(char[], char[], uint16_t);
  int gio();
  int phut();
  int giay();

  private:
  String _gio, _phut, _giay;
  const unsigned long HTTP_TIMEOUT = 5000; // max respone time from server
};
#endif

