#include <string>
#include <vector>


struct VpnInfo {
private:
	std::wstring name;
	void* handle;
	enum class State { Disconnected, Connecting, Connected} state;
private:
	VpnInfo(std::wstring name) : name(name), handle(nullptr), state(State::Disconnected) {}
	VpnInfo(std::wstring name, void* handle) : name(name), handle(handle), state((Check(), state)) {}
public:
	VpnInfo(VpnInfo&& info) noexcept : name(info.name), handle(info.handle), state(info.state) {}
public:
	const std::wstring& GetName() const { return name; }
	bool IsConnected() const { return state == State::Connected; }
	bool IsDisconnected() const { return state == State::Disconnected; }
public:
	void Connect();
	void Check();
	void Disconnect();
public:
	static std::vector<VpnInfo> Enumerate();
};
