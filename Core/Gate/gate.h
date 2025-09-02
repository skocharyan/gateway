#ifndef GATE_H
#define GATE_H


#ifdef __cplusplus
extern "C" {
#endif



    typedef  enum {
        NONE,
        OPEN,
        CLOSE
    } GateAction;

    typedef enum {
        IDLE,
        OPENING,
        CLOSING,
        OPENED,
        CLOSED
    }GateState;


    void handleGate(GateAction action);

    GateState getGateActualState(void);

    GateState getGateState(void);

#ifdef __cplusplus
}
#endif

#endif