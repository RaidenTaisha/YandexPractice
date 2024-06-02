#include <iostream>

using namespace std;

class TV {
public:
  void
  TurnOn()
  {
    state_ = true;
  }

  void
  TurnOff()
  {
    state_ = false;
  }

  bool
  IsTurnedOn()
  {
    return state_;
  }

  int
  GetCurrentChannel()
  {
    int ret = 0;
    if (state_) {
      ret = channel_;
    }
    return ret;
  }

  bool
  SelectChannel(int channel)
  {
    bool ret = false;
    if (channel > 0 && channel < 100 && state_) {
      channel_ = channel;
      ret = true;
    }
    return ret;
  }

private:
  bool state_ = false;
  int channel_ = 1;
};

int main() {
  TV tv;
  cout << tv.IsTurnedOn() << ", channel:"s << tv.GetCurrentChannel() << endl;
  tv.TurnOn();
  cout << tv.IsTurnedOn() << ", channel: "s << tv.GetCurrentChannel() << endl;
  tv.SelectChannel(25);
  cout << tv.IsTurnedOn() << ", channel: "s << tv.GetCurrentChannel() << endl;
  tv.TurnOff();
  cout << tv.IsTurnedOn() << ", channel: "s << tv.GetCurrentChannel() << endl;
}
