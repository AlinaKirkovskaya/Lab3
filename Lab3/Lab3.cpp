#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <random>
#include <limits>

class Process {
public:
    int id;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int waiting_time;
    int original_priority;

    Process(int _id, int _arrival, int _burst, int _priority)
        : id(_id),
        arrival_time(_arrival),
        burst_time(_burst),
        priority(_priority),
        remaining_time(_burst),
        waiting_time(0),
        original_priority(_priority) {
    }
};

class ProcessScheduler {
private:
    std::vector<Process> processes;
    std::mt19937 rng;

public:
    ProcessScheduler() : rng(std::random_device()()) {
        std::uniform_int_distribution<int> arrival_dist(0, 10);
        std::uniform_int_distribution<int> burst_dist(1, 20);
        std::uniform_int_distribution<int> priority_dist(1, 5);

        // Створення 6 процесів з випадковими параметрами
        for (int i = 0; i < 6; ++i) {
            processes.emplace_back(
                i + 1,
                arrival_dist(rng),
                burst_dist(rng),
                priority_dist(rng)
            );
        }
    }

    // Метод для виведення інформації про процеси
    void printProcesses() {
        std::cout << "Процеси:\n";
        std::cout << std::setw(5) << "ID"
            << std::setw(15) << "Час прибуття"
            << std::setw(15) << "Час виконання"
            << std::setw(15) << "Пріоритет\n";

        for (const auto& p : processes) {
            std::cout << std::setw(5) << p.id
                << std::setw(15) << p.arrival_time
                << std::setw(15) << p.burst_time
                << std::setw(15) << p.priority << std::endl;
        }
        std::cout << std::endl;
    }

    // Алгоритм Shortest Job First (SJF)
    void sjfScheduling() {
        std::cout << "Алгоритм Shortest Job First (SJF)\n";
        std::vector<Process> sjf_processes = processes;

        // Допоміжна структура для відстеження виконання процесів
        std::vector<Process*> ready_queue;
        int current_time = 0;
        double total_waiting_time = 0;

        // Впорядкування процесів
        std::sort(sjf_processes.begin(), sjf_processes.end(),
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });

        while (!sjf_processes.empty() || !ready_queue.empty()) {
            // Додаємо процеси, що прибули до поточного часу
            for (auto it = sjf_processes.begin(); it != sjf_processes.end();) {
                if (it->arrival_time <= current_time) {
                    ready_queue.push_back(&(*it));
                    it = sjf_processes.erase(it);
                }
                else {
                    ++it;
                }
            }

            // Якщо черга порожня, просуваємо час
            if (ready_queue.empty()) {
                current_time++;
                continue;
            }

            // Вибираємо процес з найменшим часом виконання
            auto shortest_job = std::min_element(ready_queue.begin(), ready_queue.end(),
                [](const Process* a, const Process* b) {
                    return a->burst_time < b->burst_time;
                });

            Process* current_process = *shortest_job;
            ready_queue.erase(shortest_job);

            // Оновлюємо час очікування для інших процесів у черзі
            for (auto* p : ready_queue) {
                p->waiting_time += current_process->burst_time;
            }

            // Виконання процесу
            int start_time = current_time;
            current_time += current_process->burst_time;
            int completion_time = current_time;
            int waiting_time = start_time - current_process->arrival_time;

            total_waiting_time += waiting_time;

            std::cout << "Процес " << current_process->id
                << ": Початок - " << start_time
                << ", Завершення - " << completion_time
                << ", Очікування - " << waiting_time << std::endl;
        }

        std::cout << "Середній час очікування: "
            << total_waiting_time / processes.size() << std::endl;
        std::cout << std::endl;
    }

    // Алгоритм пріоритетного планування з механізмом старіння
    void prioritySchedulingWithAging() {
        std::cout << "Пріоритетне планування з механізмом старіння\n";
        std::vector<Process> priority_processes = processes;

        // Налаштування механізму старіння
        const int AGING_THRESHOLD = 5;  // Час, після якого починається підвищення пріоритету
        const int MAX_PRIORITY_BOOST = 3;  // Максимальне підвищення пріоритету

        int current_time = 0;
        std::vector<Process*> ready_queue;

        // Впорядкування процесів за часом прибуття
        std::sort(priority_processes.begin(), priority_processes.end(),
            [](const Process& a, const Process& b) {
                return a.arrival_time < b.arrival_time;
            });

        while (!priority_processes.empty() || !ready_queue.empty()) {
            // Додаємо процеси, що прибули до поточного часу
            for (auto it = priority_processes.begin(); it != priority_processes.end();) {
                if (it->arrival_time <= current_time) {
                    ready_queue.push_back(&(*it));
                    it = priority_processes.erase(it);
                }
                else {
                    ++it;
                }
            }

            // Якщо черга порожня, просуваємо час
            if (ready_queue.empty()) {
                current_time++;
                continue;
            }

            // Механізм старіння: підвищення пріоритету для процесів, що чекають
            for (auto* p : ready_queue) {
                // Підвищуємо пріоритет, якщо процес чекає більше певного часу
                int waiting_time = current_time - p->arrival_time;
                if (waiting_time > AGING_THRESHOLD) {
                    // Обмежуємо максимальне підвищення пріоритету
                    int priority_boost = std::min((waiting_time - AGING_THRESHOLD) / 2, MAX_PRIORITY_BOOST);
                    p->priority = std::max(1, p->original_priority - priority_boost);
                }
            }

            // Вибираємо процес з найвищим пріоритетом (найменше число)
            auto highest_priority_process = std::min_element(ready_queue.begin(), ready_queue.end(),
                [](const Process* a, const Process* b) {
                    return a->priority < b->priority;
                });

            Process* current_process = *highest_priority_process;
            ready_queue.erase(highest_priority_process);

            // Виконання процесу
            std::cout << "Процес " << current_process->id
                << ": Orig. пріоритет - " << current_process->original_priority
                << ", Поточний пріоритет - " << current_process->priority
                << ", Час виконання - " << current_process->burst_time
                << ", Час очікування - " << current_time - current_process->arrival_time
                << std::endl;

            current_time += current_process->burst_time;
        }
        std::cout << std::endl;
    }

    void runSimulation() {
        printProcesses();
        sjfScheduling();
        prioritySchedulingWithAging();
    }
};

#include <iostream>

int main()
{
    ProcessScheduler scheduler;
    scheduler.runSimulation();
    return 0;
}

