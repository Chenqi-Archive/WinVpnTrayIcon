#include <string>
#include <vector>
#include <functional>


class VpnInfo {
public:
	enum class State { Disconnected, Connecting, Connected };
private:
	std::wstring name;
	void* handle;
	State state;
private:
	VpnInfo(std::wstring name, void* handle = nullptr);
public:
	VpnInfo(VpnInfo&& info) noexcept : name(std::move(info.name)), handle(info.handle), state(info.state) { info.handle = nullptr; }
public:
	const std::wstring& GetName() const { return name; }
	State GetState() const { return state; }
private:
	friend void SetVpnInfoState(VpnInfo&, State);
public:
	void Connect(std::function<void()> callback);
	void Disconnect();
public:
	static std::vector<VpnInfo> Enumerate();
};
