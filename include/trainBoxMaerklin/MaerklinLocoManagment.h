#pragma once

#include <Arduino.h>
#include "trainBoxMaerklin/MaerklinConfigDataStream.h"
#include "miniz.h"

// ToDo:
// handle different files that are received. Currently only locoInfo is used

class MaerklinLocoManagment : public MaerklinConfigDataStream
{
    public:
        struct LocoData
        {
            std::array<uint8_t, 16> name;
            std::vector<uint8_t> config;
        };

        enum class LocoManagmentState : uint8_t
        {
            Idle,
            WaitingForLocoList,
            WaitingForLocoNamen,
            WaitingForLocoInfo
        };

    public:
        // can interface is needed to request data over interface
        MaerklinLocoManagment(uint32_t uid, MaerklinCanInterface& interface, std::vector<MaerklinStationConfig>& stationList, unsigned long messageTimeout, uint8_t maxCmdRepeat);
        virtual ~MaerklinLocoManagment();

        void cyclic();

        uint32_t getUid();

        void getAllLocos(std::vector<uint8_t>& locoList, std::vector<std::unique_ptr<LocoData>> &locos, void (*callback)(bool success));

        

    protected:        

        // function is called by class ConfigDataStream in case that values where successful received with or without intention
        // or a planed transmission failed

        void m_reportResultFunc(std::vector<uint8_t>* data, uint16_t hash, bool success)override 
        {handleConfigDataStreamFeedback(data, hash, success);}
        
        void handleConfigDataStreamFeedback(std::vector<uint8_t>* data, uint16_t hash, bool success);

        void newLocoList(std::vector<uint8_t>& locoList);

        // ich muss den aktuellen Buffer jeweils umschalten => hierzu wird Minimum eine Statemaschine benötigt, welche kontrolliert,
        // was als nächstes gedownloaded wird
        // sobald die locolist gedownloaded ist, werden alle lokomotivnamen extahiert und gespeichert.
        // Anschließend wird jeweils mit push_back ein speicher angelegt, welcher dann request ConfigData übergeben wird
        
        std::vector<std::unique_ptr<LocoData>>* m_locos {nullptr};

        uint32_t m_uid {0};

        LocoManagmentState m_state{LocoManagmentState::WaitingForLocoList};

        unsigned long m_lastCmdTimeINms {0};

        unsigned long m_cmdTimeoutINms{0};

        uint8_t m_maxCmdRepeat{1};

        uint8_t m_cmdRepeat{0};

        DataType m_lastType{DataType::Lokliste};

        char m_lastInfo[16];

        std::vector<uint8_t>* m_lastBuffer{nullptr};

        bool m_isZLib {false};

        uint8_t m_numberOfLoco{0};

        uint8_t m_transmissionStarted{false};

        void (*m_callbackFunc)(bool success){nullptr};
};