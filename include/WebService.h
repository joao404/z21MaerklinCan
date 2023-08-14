#pragma once

#include <WebServer.h>
#include <AutoConnect.h>
#include <SPIFFS.h>
#include <functional>
#include <string>
#include <memory>

//#define DEBUG

// class is designed as a singelton
class WebService
{
public:
    static WebService *getInstance();
    virtual ~WebService();

    void cyclic();

    void begin(AutoConnectConfig &autoConnectConfig, std::function<void(void)> deleteLocoConfigFkt, std::function<void(void)> defaultLocoListFkt,
               std::function<void(bool)> programmingFkt, std::function<void(void)> readingFkt, std::function<void(void)> searchMotorolaFkt,
               std::function<void(void)> searchDccShortFkt, std::function<void(void)> searchDccLongFkt, std::string* foundLocoString);

    void setLokomotiveAvailable(bool isAvailable);
    void setTransmissionFinished(bool hasFinished);

    void setLocoList(std::vector<std::string> *locoList) { m_locoList = locoList; };

    AutoConnect& getAutoConnect() {return m_AutoConnect;};

private:
    static WebService *m_instance;
    WebService();
    static void handleNotFound(void);
    static String postUpload(AutoConnectAux &aux, PageArgument &args);
    String getContentType(const String &filename);

    std::function<void(void)> m_deleteLocoConfigFkt;
    std::function<void(void)> m_defaultLocoListFkt;
    std::function<void(bool)> m_programmingFkt;
    std::function<void(void)> m_readingFkt;

    bool m_lokomotiveAvailable{true};
    bool m_transmissionFinished{true};

    std::vector<std::string> *m_locoList{nullptr};

    std::function<void(void)> m_searchMotorolaFkt;
    std::function<void(void)> m_searchDccShortFkt;
    std::function<void(void)> m_searchDccLongFkt;

    std::string* m_foundLocoString{nullptr};

    WebServer m_WebServer;
    AutoConnect m_AutoConnect;
    AutoConnectAux m_auxZ60Config;
    AutoConnectCheckbox m_deleteLocoConfig;
    AutoConnectCheckbox m_defaultLocoCs2;
    AutoConnectCheckbox m_progActive;
    AutoConnectCheckbox m_readingLoco;
    AutoConnectSubmit m_saveButton;
    AutoConnectSubmit m_getZ21DbButton;

    AutoConnectAux m_auxZ60ConfigStatus;
    AutoConnectText m_readingStatus;
    AutoConnectText m_locoNames;
    AutoConnectSubmit m_reloadButton;

    AutoConnectAux m_auxZ60Upload;
    AutoConnectFile m_uploadFile;
    AutoConnectSubmit m_uploadButton;

    AutoConnectAux m_auxZ60UploadStatus;

    AutoConnectAux m_auxLocoSearch;
    AutoConnectCheckbox m_motorolaActive;
    AutoConnectCheckbox m_dccShortActive;
    AutoConnectCheckbox m_dccLongActive;
    AutoConnectSubmit m_triggerLocoSearchButton;
    AutoConnectText m_foundLoco;
};