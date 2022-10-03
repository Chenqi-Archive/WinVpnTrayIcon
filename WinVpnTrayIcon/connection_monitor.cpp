#include "connection_monitor.h"

#include "WndDesign/message/timer.h"

#include <list>
#include <functional>


BEGIN_NAMESPACE(Anonymous)

using namespace WndDesign;

class TaskQueue {
private:
	static constexpr uint timer_period = 1000;
private:
	Timer timer = Timer([&]() { OnTimer(); });
	std::list<std::pair<void*, std::function<void()>>> queue;
private:
	void OnTimer() {
		for (auto it = queue.begin(); it != queue.end();) {
			if (it->second == nullptr) {
				queue.erase(it++);
			} else {
				(it++)->second();
			}
		}
		if (queue.empty()) {
			timer.Stop();
		}
	}
public:
	void AddTask(void* index, std::function<void()> callback) {
		queue.emplace_back(index, callback);
		if (!timer.IsSet()) {
			timer.Set(timer_period);
		}
	}
	void RemoveTask(void* index) {
		for (auto& task : queue) {
			if (task.first == index) {
				task.second = nullptr;
			}
		}
	}
};

TaskQueue task_queue;

END_NAMESPACE(Anonymous)


ConnectionMonitor::ConnectionMonitor(VpnInfo&& connection, std::function<void()> callback) :
	VpnInfo(std::move(connection)), callback(callback) {
}

ConnectionMonitor::~ConnectionMonitor() { task_queue.RemoveTask(this); }

void ConnectionMonitor::Connect() {
	VpnInfo::Connect();
	wait_second = 0;
	task_queue.AddTask(this, [&]() {
		if (Check(); IsConnected() || ++wait_second >= max_wait_second) {
			return callback(), task_queue.RemoveTask(this);
		}
	});
}

void ConnectionMonitor::Disconnect() {
	VpnInfo::Disconnect();
	wait_second = 0;
	task_queue.AddTask(this, [&]() {
		if (Check(); IsDisconnected() || ++wait_second >= max_wait_second) {
			return callback(), task_queue.RemoveTask(this);
		}
	});
}
