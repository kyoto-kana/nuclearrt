#pragma once

#include <vector>
#include <algorithm>
#include <string>

enum class TimerEventType {
    Every,
    Equals,
    GreaterThan,
    LessThan
};

class ScheduledTimerEvent {
public:
    std::string name;
    int remaining = 0;
    int initialDelay = 0;
    int interval = 0;
    int nextFireTime = 0;
    int index = 0;
    bool trackEventIndex = false;
};

class TimerEvent {
public:
    int id; // ID of the event
    int time; // Total Time in milliseconds of this event
    int checkTime; // Time to check for the event
    TimerEventType type; // Type of event
    bool triggered; // Whether the condition has been met
    bool alreadyChecked; // Whether the event has already been checked this frame

    TimerEvent(int id, int checkTime, TimerEventType type) 
        : id(id), time(0), checkTime(checkTime), type(type), 
          triggered(false), alreadyChecked(false) {}

    void Update(int deltaTime) {
        time += deltaTime;
    }

    void Reset() {
        time = 0;
        triggered = false;
        alreadyChecked = false;
    }
};

class Timer {
private:
    int time;
    int deltaTime;
    int eventIndex;
    std::vector<TimerEvent> events;
    std::vector<ScheduledTimerEvent> namedEvents;

public:
    Timer() : time(0), deltaTime(0), eventIndex(0) {}

    void Update(double dt) {
        deltaTime = static_cast<int>(dt * 1000.0 + (dt >= 0.0 ? 0.5 : -0.5));
        if (deltaTime < 0) {
            deltaTime = 0;
        }
        time += deltaTime;

        // Update event times
        for (auto& evt : events) {
            evt.alreadyChecked = false;
            evt.Update(deltaTime);
        }
    }

    void SetTime(int time) {
        this->time = time;
    }

    int GetTime() const {
        return time;
    }

    int GetHundreds() const {
        return (time / 10) % 100;
    }

    int GetSeconds() const {
        return (time / 1000) % 60;
    }

    int GetMinutes() const {
        return (time / 60000) % 60;
    }

    int GetHours() const {
        return (time / 3600000) % 24;
    }

    int GetEventIndex() const {
        return eventIndex;
    }

    void StartTimerEvent(const std::string& name, int count, int initialDelayMs, int intervalMs, bool trackEventIndex = false) {
        if (count <= 0) return;

        if (initialDelayMs < 0) initialDelayMs = 0;
        if (intervalMs < 0) intervalMs = 0;

        ScheduledTimerEvent evt;
        evt.name = name;
        evt.remaining = count;
        evt.initialDelay = initialDelayMs;
        evt.interval = intervalMs;
        evt.nextFireTime = time + initialDelayMs;
        evt.index = 0;
        evt.trackEventIndex = trackEventIndex;
        namedEvents.push_back(evt);
    }

    template<typename Callback>
    void ProcessNamedEvents(Callback&& onEvent) {
        for (int i = 0; i < namedEvents.size(); i++)
        {
            auto& evt = namedEvents[i];
            if (time < evt.nextFireTime) continue;

            if (evt.trackEventIndex) {
                eventIndex = evt.index++;
            }

            onEvent(evt.name);

            namedEvents[i].remaining--;
            if (evt.remaining > 0) {
                evt.nextFireTime += evt.interval;
            }
        }

        namedEvents.erase(
            std::remove_if(namedEvents.begin(), namedEvents.end(),
                [](const ScheduledTimerEvent& e) { return e.remaining <= 0; }),
            namedEvents.end());
    }

    bool CheckEvent(int evtID, int checkTime, TimerEventType type) {
        // Check if event exists, if it does not, add it
        auto it = std::find_if(events.begin(), events.end(), 
            [evtID](const TimerEvent& evt) { return evt.id == evtID; });

        if (it == events.end()) {
            events.emplace_back(evtID, checkTime, type);
            events.back().Update(deltaTime);
            it = events.end() - 1;
        }

        TimerEvent& evt = *it;

        if (evt.type == TimerEventType::Equals) {
            if (time < evt.checkTime) {
                evt.triggered = false;
                evt.alreadyChecked = false;
            }
        }

        // Check if the event has already been checked this frame
        if (evt.alreadyChecked) {
            return false;
        }

        // Check conditions
        switch (type) {
            case TimerEventType::Every:
                if (evt.checkTime <= 0 || evt.time >= evt.checkTime) {
                    if (evt.checkTime > 0) {
                        evt.time %= evt.checkTime;
                    }
                    evt.triggered = true;
                    evt.alreadyChecked = true;
                    return true;
                }
                break;
            case TimerEventType::Equals:
                if (time >= evt.checkTime && !evt.triggered) {
                    evt.triggered = true;
                    evt.alreadyChecked = true;
                    return true;
                }
                break;
            case TimerEventType::GreaterThan:
                if (time > evt.checkTime) {
                    evt.triggered = true;
                    return true;
                }
                break;
            case TimerEventType::LessThan:
                if (time < evt.checkTime) {
                    evt.triggered = true;
                    return true;
                }
                break;
        }

        return false;
    }
}; 