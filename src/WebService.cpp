#include "WebService.h"
#include <map>
#include <ESPmDNS.h>

WebService *WebService::m_instance{nullptr};

WebService::WebService()
    : m_AutoConnect(m_WebServer),
      m_auxZ60Config("/", "Config"),
      m_deleteLocoConfig("deleteLocoConfig", "deleteLocoConfig", "Delete internal memory for z21 loco config", false),
      m_defaultLocoCs2("defaultLocoCs2", "defaultLocoCs2", "Set lokomotive.cs2 back to default values", false),
      m_progActive("progActive", "progActive", "Trackprogramming activ", false),
      m_readingLoco("readingLoco", "readingLoco", "Read locos from Mobile Station", false),
      m_saveButton("saveButton", "Run", "/z60configstatus"),
      m_getZ21DbButton("getZ21DbButton", "Download Z21 Database", "/z21.html"),
      m_auxZ60ConfigStatus("/z60configstatus", "Config Status"),
      m_readingStatus("readingStatus", "readingStatus", "Reading locos: %s"),
      m_locoNames("locoNames", "locoNames", "%s"),
      m_reloadButton("realoadButton", "Reload", "/z60configstatus"),
      m_auxZ60Upload("/upload", "File Upload"),
      m_uploadFile("uploadFile", "uploadFile", "Select file:"),
      m_uploadButton("uploadButton", "Upload", "/uploadstatus"),
      m_auxZ60UploadStatus("/uploadstatus", "Upload Status"),
      m_auxLocoSearch("/locosearch", "Loco search"),
      m_motorolaActive("searchMotorola", "searchMotorola", "Search for Motorola", false),
      m_dccShortActive("searchDccShort", "searchDccShort", "Search for DCC short", false),
      m_dccLongActive("searchDccLong", "searchDccLong", "Search for DCC long", false),
      m_triggerLocoSearchButton("triggerSearchButton", "Start", "/locosearch"),
      m_foundLoco("foundLoco", "foundLoco", "%s")
{
    m_WebServer.on("/can", [this]()
                   {
                        Serial.println("Can requested");
                        m_WebServer.send(200, "", ""); });

    m_WebServer.on("/config/prefs.cs2", [this]()
                   {
                        Serial.println("prefs requested");
                        m_WebServer.send(200, "text/plain", 
                        F(
                            "[Preferences]\nversion\n .major=0\n .minor=1\npage\n .entry\n ..key=Version\n ..value=\n"
                            "page\n .entry\n ..key=SerNum\n ..value=84\n .entry\n ..key=GfpUid\n ..value=1129525928\n .entry\n ..key=GuiUid\n"
                            " ..value=1129525929\n .entry\n ..key=HardVers\n ..value=3.1\n"
                        )); });

    m_WebServer.on("/config/magnetartikel.cs2", [this]()
                   {
                        Serial.println("magnetartikel requested");
                        m_WebServer.send(200, "text/plain",
                        F(
                            "[magnetartikel]\n"
                            "version\n"
                            " .minor=1\n"
                        )); });

    m_WebServer.on("/config/gleisbild.cs2", [this]()
                   {
                        Serial.println("gleisbild requested");
                        m_WebServer.send(200, "text/plain",
                        F(
                            "[gleisbild]\n"
                            "version\n"
                            " .major=1\n"
                            "groesse\n"
                            "zuletztBenutzt\n"
                            " .name=gleisbildDummy\n"
                            "seite\n"
                            " .name=gleisbildDummy\n"
                        )); });

    m_WebServer.on("/config/fahrstrassen.cs2", [this]()
                   {
                        Serial.println("fahrstrassen requested");
                        m_WebServer.send(200, "text/plain",
                        F(
                            "[fahrstrassen]\n"
                            "version\n"
                            " .minor=4\n"
                        )); });

    m_WebServer.on("/config/gleisbilder/gleisbildDummy.cs2", [this]()
                   {
                        Serial.println("gleisbildDummy requested");
                        m_WebServer.send(200, "text/plain",
                        F(
                            "[gleisbildseite]\n"
                            "version\n"
                            " .major=1\n"
                        )); });

    m_WebServer.on("/config/geraet.vrs", [this]()
                   {
                        Serial.println("geraet requested");
                        m_WebServer.send(200, "text/plain",
                        F(
                            "[geraet]\n"
                            "version\n"
                            " .minor=1\n"
                            "geraet\n"
                            " .sernum=1\n"
                            " .hardvers=ESP,1\n"
                            ""
                        )); });

    // m_WebServer.on("/ajaxlokliste", [this]()
    //                {
    //                     Serial.println("ajaxlokliste requested");
    //                     std::string xml = "<?xml version='1.0'?>";
    //                     xml += "<xml>";
    //                     if(nullptr != m_locoList)
    //                     {
    //                         int i = 0;
    //                         for (auto iterator = m_locoList->begin(); iterator != m_locoList->end(); iterator++, i++)
    //                         {
    //                             xml += "<loco" + std::to_string(i) + ">";
    //                             xml += iterator->c_str();
    //                             xml += "</loco" + std::to_string(i) + ">";
    //                         }
    //                     }
    //                     xml += "</xml>";
    //                     m_WebServer.send(200, "text/plain", xml.c_str()); });
}

WebService *WebService::getInstance()
{
    if (nullptr == m_instance)
    {
        m_instance = new WebService();
    }
    return m_instance;
};

WebService::~WebService()
{
}

void WebService::cyclic()
{
    m_AutoConnect.handleClient();
}

void WebService::begin(AutoConnectConfig &autoConnectConfig, std::function<void(void)> deleteLocoConfigFkt, std::function<void(void)> defaultLocoListFkt,
                       std::function<void(bool)> programmingFkt, std::function<void(void)> readingFkt, std::function<void(void)> searchMotorolaFkt,
                       std::function<void(void)> searchDccShortFkt, std::function<void(void)> searchDccLongFkt, std::string* foundLocoString)
{
    m_programmingFkt = programmingFkt;

    m_readingFkt = readingFkt;

    m_defaultLocoListFkt = defaultLocoListFkt;

    m_deleteLocoConfigFkt = deleteLocoConfigFkt;

    m_searchMotorolaFkt = searchMotorolaFkt;

    m_searchDccShortFkt = searchDccShortFkt;

    m_searchDccLongFkt = searchDccLongFkt;

    m_foundLocoString = foundLocoString;

    m_auxZ60ConfigStatus.on([this](AutoConnectAux &aux, PageArgument &arg)
                            {
                                if (m_AutoConnect.where() != "/z60configstatus") 
                                {
                                    if (m_WebServer.hasArg("deleteLocoConfig"))
                                    {
                                        Serial.println("deleting z21 loco config");
                                        m_deleteLocoConfigFkt();
                                    }
                                    if (m_WebServer.hasArg("defaultLocoCs2"))
                                    {
                                        Serial.println("default loco list");
                                        m_defaultLocoListFkt();
                                    }
                                    if (m_WebServer.hasArg("progActive"))
                                    {
                                        Serial.println("setProgramming(true)");
                                        m_programmingFkt(true);
                                    }
                                    else
                                    {
                                        Serial.println("setProgramming(false)");
                                        m_programmingFkt(false);
                                    }
                                    if (m_WebServer.hasArg("readingLoco"))
                                    {
                                        Serial.println("trigger loco reading");
                                        m_readingFkt();
                                    }
                                }
                            if(m_transmissionFinished)
                            {
                                aux["readingStatus"].value = "Finished with";
                                aux["readingStatus"].value += m_lokomotiveAvailable ? "Success" : "Failure";
                            }
                            else
                            {
                                aux["readingStatus"].value = "Running";
                            }
                            if(nullptr != m_locoList)
                                {
                                    aux["locoNames"].value = "";
                                    for(auto loco : *m_locoList)
                                    {
                                        aux["locoNames"].value += loco.c_str();
                                        aux["locoNames"].value += "/";
                                    }
                                }
                                else
                                {
                                    aux["locoNames"].value = "nullptr";
                                }
                            
                            return String(); });

    m_AutoConnect.config(autoConnectConfig);

    m_AutoConnect.onNotFound(WebService::handleNotFound);

    m_auxZ60Config.add({m_deleteLocoConfig, m_defaultLocoCs2, m_progActive, m_readingLoco, m_saveButton, m_getZ21DbButton});
    m_auxZ60ConfigStatus.add({m_readingStatus, m_locoNames, m_reloadButton, m_getZ21DbButton});

    m_AutoConnect.join(m_auxZ60Config);
    m_AutoConnect.join(m_auxZ60ConfigStatus);

    m_auxZ60Upload.add({m_uploadFile, m_uploadButton});
    m_auxZ60UploadStatus.add({m_readingStatus, m_locoNames, m_getZ21DbButton});

    m_AutoConnect.join(m_auxZ60Upload);
    m_AutoConnect.join(m_auxZ60UploadStatus);

    m_auxZ60UploadStatus.on(WebService::postUpload);

    // m_AutoConnect.append("/z21.html", "z21DB");

    m_auxLocoSearch.add({m_motorolaActive, m_dccShortActive, m_dccLongActive, m_triggerLocoSearchButton, m_foundLoco});

    m_AutoConnect.join(m_auxLocoSearch);

    m_auxLocoSearch.on([this](AutoConnectAux &aux, PageArgument &arg)
                       {
                                    if (m_WebServer.hasArg("searchMotorola"))
                                    {
                                        Serial.println("search motorola");
                                        m_searchMotorolaFkt();
                                    }
                                    else if (m_WebServer.hasArg("searchDccShort"))
                                    {
                                        Serial.println("search dcc short");
                                        m_searchDccShortFkt();
                                    }
                                    else if (m_WebServer.hasArg("searchDccLong"))
                                    {
                                        Serial.println("search dcc long");
                                        m_searchDccLongFkt();
                                    }

                            if(nullptr != m_foundLocoString)
                            {
                                aux["foundLoco"].value = m_foundLocoString->c_str();
                            }
                            else
                            {
                                aux["foundLoco"].value = "Nothing searched yet";
                            }
                            
                            return String(); });

    m_AutoConnect.begin();

    if (MDNS.begin("gleisbox"))
    {
        MDNS.addService("http", "tcp", 80);
    }
}

void WebService::setLokomotiveAvailable(bool isAvailable)
{
    m_lokomotiveAvailable = isAvailable;
}

void WebService::setTransmissionFinished(bool hasFinished)
{
    m_transmissionFinished = hasFinished;
}

void WebService::handleNotFound(void)
{
    const String filePath = m_instance->m_WebServer.uri();
    // Serial.print(filePath);
    // Serial.println(" requested");

    if (SPIFFS.exists(filePath.c_str()))
    {
        if (strcmp("/config/lokomotive.cs2", filePath.c_str()) == 0)
        {
            if (!m_instance->m_lokomotiveAvailable)
            {
                m_instance->m_WebServer.send(404, "text/plain", "lokomotive.cs2 under construction");
                return;
            }
        }
        File uploadedFile = SPIFFS.open(filePath.c_str(), "r");
        String mime = m_instance->getContentType(filePath);
        m_instance->m_WebServer.streamFile(uploadedFile, mime);
        uploadedFile.close();
    }
    else if (filePath.startsWith("/images/cs2/fcticons"))
    {
        String requestedFile = filePath.substring(11);
        if (SPIFFS.exists(requestedFile.c_str()))
        {
            File uploadedFile = SPIFFS.open(requestedFile.c_str(), "r");
            String mime = m_instance->getContentType(requestedFile);
            m_instance->m_WebServer.streamFile(uploadedFile, mime);
            uploadedFile.close();
        }
        else
        {
            m_instance->m_WebServer.send(404, "text/plain", "png not available");
        }
    }
    else if (filePath.endsWith(".png"))
    {
        // send default picture if no picture is available
        const char *requestedFile{"/default.png"};
        if (SPIFFS.exists(requestedFile))
        {
            File uploadedFile = SPIFFS.open(requestedFile, "r");
            String mime = m_instance->getContentType(requestedFile);
            m_instance->m_WebServer.streamFile(uploadedFile, mime);
            uploadedFile.close();
        }
        else
        {
            m_instance->m_WebServer.send(404, "text/plain", (filePath + " not available").c_str());
        }
    }
    else
    {
        String message = "File Not Found\n";
        message += "URI: ";
        message += m_instance->m_WebServer.uri();
        message += "\nMethod: ";
        message += (m_instance->m_WebServer.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += m_instance->m_WebServer.args();
        message += "\n";
        for (uint8_t i = 0; i < m_instance->m_WebServer.args(); i++)
        {
            message += " " + m_instance->m_WebServer.argName(i) + ": " + m_instance->m_WebServer.arg(i) + "\n";
        }
#ifdef DEBUG
        Serial.print(message);
#endif
        m_instance->m_WebServer.send(404, "text/plain", message);
    }
}

String WebService::postUpload(AutoConnectAux &aux, PageArgument &args)
{
    String result{"Uploaded" + m_instance->m_uploadFile.value};
    AutoConnectFile &upload{m_instance->m_uploadFile};
    Serial.printf("Uploaded: %s\n", upload.value.c_str());
    if (SPIFFS.exists(String("/" + upload.value).c_str()))
    {
        if (SPIFFS.exists("/config/lokomotive.cs2"))
        {
            SPIFFS.remove("/config/lokomotive.cs2");
        }
        SPIFFS.rename(String("/" + upload.value).c_str(), "/config/lokomotive.cs2");
    }
    else
    {
        result = "Not saved";
    }
    m_instance->m_readingStatus.value = result;
    return String();
}

String WebService::getContentType(const String &filename)
{
    const static std::map<const String, const String> contentTypes{
        {".cs2", "text/plain"},
        {".txt", "text/plain"},
        {".htm", "text/html"},
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".ico", "image/x-icon"},
        {".svg", "image/svg+xml"},
        {".xml", "text/xml"},
        {".pdf", "application/x-pdf"},
        {".zip", "application/x-zip"},
        {".gz", "application/x-gzip"},
        {".z21", "application/octet-stream"}};
    for (auto iter = contentTypes.begin(); iter != contentTypes.end(); ++iter)
    {
        if (filename.endsWith(iter->first))
        {
            return iter->second;
        }
    }
    return "text/plain";
}