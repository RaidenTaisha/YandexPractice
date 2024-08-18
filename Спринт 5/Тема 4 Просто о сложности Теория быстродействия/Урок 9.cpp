#include <algorithm>
#include <deque>
#include <string>

using namespace std;

struct Ticket {
  int id;
  string name;
};

class TicketOffice {
 public:
  // добавить билет в систему
  void PushTicket(const string& name) {
    tickets_.push_back({last_id_, name});
    ++last_id_;
  }

  // получить количество доступных билетов
  int GetAvailable() const {
    return static_cast<int>(tickets_.size());
  }

  // получить количество доступных билетов определённого типа
  int GetAvailable(const string& name) const {
    return static_cast<int>(std::count_if(tickets_.begin(),
                                          tickets_.end(),
                                          [name](const auto& ticket) {
                                            return ticket.name == name;
                                          }));
  }

  // отозвать старые билеты (до определённого id)
  void Invalidate(int minimum) {
    while (!tickets_.empty()) {
      const auto& front = tickets_.front();
      if (front.id < minimum) {
        tickets_.pop_front();
      } else {
        return;
      }
    }
  }

 private:
  int last_id_ = 0;
  deque<Ticket> tickets_;
};
