/*
This simple service, will just report in the EventViewer an event with text as datails of it "Hello World" when started, and 
"End of the World" when stopped.First when choosing a communication method between user and the services, the best way is to 
use the EventViewer, where services can report differnret types of events, including different types of messages.
In this way i choose to use the EventViewer as an communication bridge, and by this use the metrhod WriteToEventLog to write driectly 
the message in the event generated for the service, and avoid using the other complex method, in which the messages are deifiened as 
constants in an mc file, than we compile it for associating id's to them, and use directly the id's in the service code for reproting 
an message.
*/
#include <windows.h>

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
LPCWSTR               g_ServiceName = L"HelloWorldService";


void WriteToEventLog(LPCWSTR message) {
    //method to create the event in EvenetViewer

    //create an handler to the service to extract info in the event about this service
    HANDLE hEventSource = RegisterEventSourceW(NULL, g_ServiceName);
    if (hEventSource != NULL) {
        LPCWSTR strings[1] = { message };
        //create the event
        ReportEventW(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, strings, NULL);
        //delete the handler from mem
        DeregisterEventSource(hEventSource);
    }
}

void WINAPI ServiceCtrlHandler(DWORD request) {
    //method to handle an action made in the context of the service

    switch (request) {
    //handle only the stop action of the service
    case SERVICE_CONTROL_STOP:
        //set the state to pending stop of the service and publish it to the OS
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

        //create an event with the message "End of the World"
        WriteToEventLog(L"End of the World");

        //set the state to service stopped of the service and publish it to the OS
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        break;
    default:
        break;
    }
}

void WINAPI ServiceMain(DWORD argc, LPWSTR* argv) {
    //main function of the service, exectued when the service is started, responsable with initalizations

    //register the hnalder of the actions made in the cintext of the service
    g_StatusHandle = RegisterServiceCtrlHandlerW(g_ServiceName, ServiceCtrlHandler);
    if (g_StatusHandle == NULL) return;
    
    //configure the SERVICE_STATUS struct for the service, with the type of process, the state of running service, and other params
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;
    g_ServiceStatus.dwWaitHint = 0;

    //publish SERVICE_STATUS struct to the OS, to update the running service info 
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    //create an event with the message "Hello World"
    WriteToEventLog(L"Hello World");
}


int main() {
    //when sc is executed with the create arg in order to create the service(create it in the services list and store the exec path), 
    //give the entrypoint of the service, so when lancuhed it executes that fucntion 
    SERVICE_TABLE_ENTRYW ServiceTable[] = {
        { (LPWSTR)g_ServiceName, (LPSERVICE_MAIN_FUNCTIONW)ServiceMain },
        { NULL, NULL }
    };

    //publish to the OS
    StartServiceCtrlDispatcherW(ServiceTable);
    return 0;
}