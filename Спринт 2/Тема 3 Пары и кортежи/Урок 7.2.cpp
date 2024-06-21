#include <iostream>
#include <map>
#include <string>
#include <tuple>

using namespace std;

// Перечислимый тип для статуса задачи
enum class TaskStatus {
  NEW,          // новая
  IN_PROGRESS,  // в разработке
  TESTING,      // на тестировании
  DONE          // завершена
};

// Объявляем тип-синоним для map<TaskStatus, int>,
// позволяющего хранить количество задач каждого статуса
using TasksInfo = map<TaskStatus, int>;

// Принимаем словарь по значению, чтобы иметь возможность
// обращаться к отсутствующим ключам с помощью [] и получать 0,
// не меняя при этом исходный словарь.
void PrintTasksInfo(TasksInfo tasks_info) {
  cout << tasks_info[TaskStatus::NEW] << " new tasks"s
       << ", "s << tasks_info[TaskStatus::IN_PROGRESS] << " tasks in progress"s
       << ", "s << tasks_info[TaskStatus::TESTING] << " tasks are being tested"s
       << ", "s << tasks_info[TaskStatus::DONE] << " tasks are done"s << endl;
}

class TeamTasks {
 public:
  // Получить статистику по статусам задач конкретного разработчика
  const TasksInfo &GetPersonTasksInfo(const string &person) const;

  // Добавить новую задачу (в статусе NEW) для конкретного разработчика
  void AddNewTask(const string &person);

  // Обновить статусы по данному количеству задач конкретного разработчика,
  // подробности см. ниже
  tuple<TasksInfo, TasksInfo> PerformPersonTasks(const string &person,
                                                 int task_count);

 private:
  static TaskStatus Next(TaskStatus task_status) {
    return static_cast<TaskStatus>(static_cast<int>(task_status) + 1);
  }

  static TaskStatus Back(TaskStatus task_status) {
    return static_cast<TaskStatus>(static_cast<int>(task_status) - 1);
  }

  map<string, TasksInfo> person_to_tasks_info_;
};

const TasksInfo &TeamTasks::GetPersonTasksInfo(const string &person) const {
  return person_to_tasks_info_.at(person);
}

void TeamTasks::AddNewTask(const string &person) {
  person_to_tasks_info_[person][TaskStatus::NEW]++;
}

tuple<TasksInfo, TasksInfo> TeamTasks::PerformPersonTasks(const string &person,
                                                          int task_count) {
  TasksInfo updated_tasks, untouched_tasks;
  if (person_to_tasks_info_.count(person) > 0) {
    untouched_tasks = person_to_tasks_info_[person];
    untouched_tasks.erase(TaskStatus::DONE);
    for (const auto &i : {TaskStatus::NEW, TaskStatus::IN_PROGRESS,
        TaskStatus::TESTING}) {
      while (task_count > 0 && untouched_tasks[i] > 0) {
        updated_tasks[Next(i)]++;
        untouched_tasks[i]--;
        task_count--;
      }
    }
    for (const auto &[status, count] : updated_tasks) {
      person_to_tasks_info_[person][status] += count;
      person_to_tasks_info_[person][Back(status)] -= count;
    }
  }

  return {updated_tasks, untouched_tasks};
}

int main() {
  TeamTasks tasks;
  tasks.AddNewTask("Ilia"s);
  for (int i = 0; i < 3; ++i) {
    tasks.AddNewTask("Ivan"s);
  }
  cout << "Ilia's tasks: "s;
  PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"s));
  cout << "Ivan's tasks: "s;
  PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"s));

  TasksInfo updated_tasks, untouched_tasks;

  tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Ivan"s, 2);
  cout << "Updated Ivan's tasks: "s;
  PrintTasksInfo(updated_tasks);
  cout << "Untouched Ivan's tasks: "s;
  PrintTasksInfo(untouched_tasks);

  tie(updated_tasks, untouched_tasks) = tasks.PerformPersonTasks("Ivan"s, 2);
  cout << "Updated Ivan's tasks: "s;
  PrintTasksInfo(updated_tasks);
  cout << "Untouched Ivan's tasks: "s;
  PrintTasksInfo(untouched_tasks);
}

