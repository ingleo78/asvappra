package com.draico.asvappra.opencl.tools;

import com.draico.asvappra.opencl.listeners.CallbackEvent;
import com.draico.asvappra.opencl.listeners.Event;
import java.util.ArrayList;

public class CheckStatusEvents extends Thread implements Runnable {
    private ArrayList<Event> listEvents;
    private Event eventError;
    private Event eventRunning;
    private Event eventSubmitted;
    private boolean status;
    private boolean stateErrorEvent;
    private boolean isEventRunning;
    private boolean isEventSubbmitted;
    public CheckStatusEvents() {
        listEvents = new ArrayList<>();
        status = true;
        start();
    }
    public void addNewEvent(Event[] events) {
        for (Event _event : events) {
            if (_event != null && !_event.isSetStatusEvent) {
                CallbackEvent callbackEvent = new CallbackEvent() {
                    @Override
                    public void notify(Event event, Integer statusCommandExecution) {
                        for (int position = 0; position < listEvents.size(); position++) {
                            if (listEvents.get(position) == event) {
                                event.setEventStatus(statusCommandExecution);
                                event.isSetStatusEvent = true;
                                listEvents.set(position, event);
                            }
                        }
                    }
                };
                _event.setEventCallback(_event.getExecutionStatus(), callbackEvent);
                _event.setEventStatus(Event.COMMAND_EXECUTION_STATUS_QUEUED);
                listEvents.add(_event);
            } else {
                String message = "There is a problem with the CheckStatusEvents class, since you are using the addNewEvent method to add a new ";
                message += "event to analyze its behavior, but the event or the callbackEvent object or both that the previous method needs ";
                message += "are set to a null value";
                MessageError messageError = new MessageError();
                messageError.showMessage(message);
            }
        }
    }
    public Event[] removeEvent(Event event) {
        if (listEvents.size() > 0) {
            listEvents.remove(event);
            event.releaseEvent();
            return (Event[])listEvents.toArray();
        } else return null;
    }
    public void stopCheckStatus() {
        interrupt();
        status = false;
    }
    public Event getEventWithError() {
        if (stateErrorEvent) return eventError;
        else return null;
    }
    public void setAllowCheckStatusEvents() { stateErrorEvent = false; }
    public boolean isEventRunning() { return isEventRunning; }
    public boolean isEventSubbmitted() { return isEventSubbmitted; }
    public boolean isEventError() { return stateErrorEvent; }
    public Event[] getEvents() { return (Event[])listEvents.toArray(); }
    @Override public void run() {
        super.run();
        int positionEvent = 0;
        while(status) {
            try {
                if (listEvents.size() > 0) {
                    if (!stateErrorEvent) {
                        if (positionEvent >= listEvents.size()) positionEvent = 0;
                        Event event = listEvents.get(positionEvent);
                        switch (event.getExecutionStatus()) {
                            case Event.COMMAND_EXECUTION_STATUS_COMPLETE:
                                removeEvent(event);
                                eventRunning = eventSubmitted = null;
                                stateErrorEvent = false;
                                isEventRunning = false;
                                isEventSubbmitted = false;
                                positionEvent = 0;
                                break;
                            case Event.COMMAND_EXECUTION_STATUS_RUNNING:
                                eventRunning = event;
                                eventError = eventSubmitted = null;
                                isEventRunning = true;
                                isEventSubbmitted = false;
                                stateErrorEvent = false;
                                break;
                            case Event.COMMAND_EXECUTION_STATUS_SUBMITTED:
                                eventSubmitted = event;
                                eventError = eventRunning = null;
                                isEventSubbmitted = true;
                                isEventRunning = false;
                                stateErrorEvent = false;
                                break;
                            default:
                                stateErrorEvent = true;
                                isEventRunning = false;
                                isEventSubbmitted = false;
                                break;
                        }
                        positionEvent++;
                    }
                }
                Thread.sleep(20);
            } catch(Exception e) { }
        }
    }
}
