#include "ILS/ILS_StdLog.h"
#include "ILS/ILS_Defines.h"
#include <Windows.h>

class App : public Logger {
public:
    App() {
		reg(&ILogger::infOut, "app", "creating 0x%p", this);
    }

    ~App() {
		reg(&ILogger::infOut, "app", "destroying 0x%p", this);
    }

    void Run() {
        ILS_WRN(("app", "enter Run() [this=0x%p]", this) << " line #" << __LINE__);

        ILS_SECTB(Load, ("загружаем всё [this=0x%p]", this) << " line #" << __LINE__) {
            for (int i = 0; i < 3; ++i) {
                ILS_SECTBI(LoadBox, i, ("загружаем коробку [this=0x%p]", this) << " line #" << __LINE__) {
                    ILS_WRN(("app", "loading box %d [this=0x%p]", i, this) << " line #" << __LINE__);
                } ILS_SECTEI(LoadBox, i, ("загружена коробка [this=0x%p]", this) << " line #" << __LINE__);
            }
        } ILS_SECTE(Load, ("загружено всё [this=0x%p]", this) << " line #" << __LINE__);

        ILS_WRN(("app", "exit Run() [this=0x%p]", this) << " line #" << __LINE__);
    }
};

int main() {
    auto logger = std::make_shared<StdLogger>("log.txt", std::cout, std::cerr);
	logger->reg(&ILogger::wrnOut, "main", "starting %d", ::GetCurrentProcessId());
    {
        App app;
        app.setPersonalLogger(logger);
        app.Run();
    }
	logger->reg(&ILogger::wrnOut, "main", "ending %d", ::GetCurrentProcessId());

    return 0;
}
