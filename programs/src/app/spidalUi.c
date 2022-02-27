#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <gtk/gtk.h>
//#include <gtk/gtkx.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/time.h>

#include "../../config/spidal.h"
#include "../../include/check.h"
#include "../../include/appStatus.h"
#include "../../include/signalsLib.h"

#define GUI_PATH PATH "src/app/gui/"

// UI
#define GUI_FILE_PATH GUI_PATH "gui.glade"

// CSS
//#define CSS_GLOBAL_STYLE_PATH GUI_PATH "css/yaru-dark.css"
#define CSS_STYLE_PATH GUI_PATH "css/style.css"
#define CSS_TECHNICAL_STYLE_PATH GUI_PATH "css/technical.css"

// Images
#define IMG_SHUTDOWN GUI_PATH "imgs/shutdown.png"
#define IMG_NETWORK_OFF GUI_PATH "imgs/network-off.png"
#define IMG_NETWORK_ON GUI_PATH "imgs/network-on.png"
#define IMG_BTN_ON GUI_PATH "imgs/btn-active.png"
#define IMG_BTN_OFF GUI_PATH "imgs/btn-inactive.png"
#define IMG_PEDAL_ON GUI_PATH "imgs/pedal-active.png"
#define IMG_PEDAL_OFF GUI_PATH "imgs/pedal-inactive.png"

// Timers frame rate
#define GUI_TIMER_DELAY 20

// Sound exe
#define MPG123_PATH "/usr/bin/mpg123.bin"
#define COUNTDOWN_PATH GUI_PATH "sounds/countdown.mp3"
#define FALSE_START_BUZZER_PATH GUI_PATH "sounds/wrong-buzzer.mp3"

// Duration of the starter sound until "go"
#define STARTER_DURATION_SEC 2
#define STARTER_DURATION_NSEC 500000000

typedef struct {
    struct timespec timeStart;
    struct timespec timeEnd;
    struct timespec reactionTimeStart;
    GtkLabel * timerLabel;
    int t_m;
    int t_s;
    int t_ms;
    int running;
    GtkStyleContext * styleContext;
    GtkWidget * stopButton;
} timer_type;

GtkApplication * app;
GtkBuilder * builder;
GtkWidget * window;

GtkHeaderBar * headerBar;
GtkButton * backButton;

GtkImage * networkIcon;
GdkPixbuf * pixbufNetworkIconOff;
GdkPixbuf * pixbufNetworkIconOn;

GtkImage * shutDownButton;
GdkPixbuf * pixbufShutdownImage;

GtkStack * stack;

GtkImage * networkImgHome;
GtkImage * btnImg[2];
GdkPixbuf * pixbufNetworkImgHomeOff;
GdkPixbuf * pixbufNetworkImgHomeOn;
GdkPixbuf * pixbufBtnOn;
GdkPixbuf * pixbufBtnOff;

GtkImage * pedalImg[2];
GdkPixbuf * pixbufPedalOn;
GdkPixbuf * pixbufPedalOff;

GtkLabel * name1;
GtkLabel * name2;
GtkLabel * reactionTime[2];
GtkLabel * runnerReadyLabel[2];
GtkButton * startButton;
GtkButton * startButton1;
GtkButton * startButton2;

timer_type timers[2];
guint timerHandlerId;
//struct timespec timeRaceStart;
int failedRace[2];

int in;
pthread_t appThread;
pid_t soundProcess;

status_t appPage;
status_t runnerStatus[2];
status_t raceStatus;

static void setupGUI(GtkApplication * app, gpointer user_data);
void hideWidget(GtkWidget * w);
void showWidget(GtkWidget * w);

void * applicationThread();
void analyseData(char * data);

void remoteStatusChanged(int status) ;
void btnStatusChanged(int n, int status, long t_s, long t_ns);
void pedalStatusChanged(int n, int status, long t_s, long t_ns);

void initTimer(timer_type * timer);
void setTimerOnError(timer_type * timer);
void startTimer(timer_type * timer, long t_s, long t_ns);
void stopTimer(timer_type * timer, long t_s, long t_ns);
void computeTime(int * t_m, int * t_s, int * t_ms, struct timespec * timeI, struct timespec * timeF);
gboolean timerHandler(void * p);
int timeCompare(long t1_s, long t1_ns, long t2_s, long t2_ns);
int timeCompareM(int t1_m, int t1_s, int t1_ms, int t2_m, int t2_s, int t2_ms);

void initRace();
void cleanRaceGUI();
void startRace();
void endRace(int showVictory);
void failRace(int n);
void falseStart(int n);
void playStarter();
void playFalseStartBuzzer();

void setRunnerReadyGUI(int n, int ready);
void setReactionTime(int n);

void signalHandler(int sigNum);

void onWindowDestroy(GtkWidget * w);

int main(int argc, char **argv) {
    int status;

    if (argc != 2) {
        fprintf(stderr, "Not the good number of arguments\n");
        exit(EXIT_FAILURE);
    }
    in = atoi(argv[1]);

    soundProcess = 0;

    initStatus(&appPage, APP_MENU);
    initStatus(&runnerStatus[0], RUNNER_WAITING);
    initStatus(&runnerStatus[1], RUNNER_WAITING);
    initStatus(&raceStatus, RACE_WAITING);

    timerHandlerId = 0;
    timers[0].running = 0;
    timers[1].running = 0;

    installSignalHandler(SIGALRM, signalHandler);

    //app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(setupGUI), NULL);
    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return status;
}

/* ------------------------------------------------------------------------------*/
/* GUI */
/* ------------------------------------------------------------------------------*/

// setup GUI
static void setupGUI(GtkApplication * app, gpointer user_data) {

    //GtkCssProvider * globalCssProvider;
    GtkCssProvider * cssStyleProvider;
    GtkCssProvider * technicalCssProvider;

    builder = gtk_builder_new_from_file(GUI_FILE_PATH);

    // Setup Window
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtk_application_add_window(app, GTK_WINDOW(window));
    gtk_window_set_title(GTK_WINDOW(window), "Spidal");
    gtk_window_set_default_size(GTK_WINDOW(window), SCREEN_RESOLUTION_X, SCREEN_RESOLUTION_Y);

    //g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(onWindowDestroy), NULL);
	gtk_builder_connect_signals(builder, NULL);
	
	// Components
	stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
    headerBar = GTK_HEADER_BAR(gtk_builder_get_object(builder, "headerBar"));
    backButton = GTK_BUTTON(gtk_builder_get_object(builder, "backButton"));

    networkIcon = GTK_IMAGE(gtk_builder_get_object(builder, "networkIcon"));
    pixbufNetworkIconOff = gdk_pixbuf_new_from_file_at_scale(IMG_NETWORK_OFF, 20, 20, TRUE, NULL);
    pixbufNetworkIconOn = gdk_pixbuf_new_from_file_at_scale(IMG_NETWORK_ON, 20, 20, TRUE, NULL);
    gtk_image_set_from_pixbuf(networkIcon, pixbufNetworkIconOff);

    shutDownButton = GTK_IMAGE(gtk_builder_get_object(builder, "shutDownButton"));
    pixbufShutdownImage = gdk_pixbuf_new_from_file_at_scale(IMG_SHUTDOWN, 20, 20, TRUE, NULL);
    gtk_image_set_from_pixbuf(shutDownButton, pixbufShutdownImage);

    networkImgHome = GTK_IMAGE(gtk_builder_get_object(builder, "networkImgHome"));
    btnImg[0] = GTK_IMAGE(gtk_builder_get_object(builder, "btn1Img"));
    btnImg[1] = GTK_IMAGE(gtk_builder_get_object(builder, "btn2Img"));
    pixbufNetworkImgHomeOff = gdk_pixbuf_new_from_file_at_scale(IMG_NETWORK_OFF, 50, 50, TRUE, NULL);
    pixbufNetworkImgHomeOn = gdk_pixbuf_new_from_file_at_scale(IMG_NETWORK_ON, 50, 50, TRUE, NULL);
    pixbufBtnOn = gdk_pixbuf_new_from_file_at_scale(IMG_BTN_ON, 40, 40, TRUE, NULL);
    pixbufBtnOff =gdk_pixbuf_new_from_file_at_scale(IMG_BTN_OFF, 40, 40, TRUE, NULL);
    gtk_image_set_from_pixbuf(networkImgHome, pixbufNetworkImgHomeOff);
    gtk_image_set_from_pixbuf(btnImg[0], pixbufBtnOff);
    gtk_image_set_from_pixbuf(btnImg[1], pixbufBtnOff);

    pedalImg[0] = GTK_IMAGE(gtk_builder_get_object(builder, "pedal1Img"));
    pedalImg[1] = GTK_IMAGE(gtk_builder_get_object(builder, "pedal2Img"));
    pixbufPedalOn = gdk_pixbuf_new_from_file_at_scale(IMG_PEDAL_ON, 60, 60, TRUE, NULL);
    pixbufPedalOff =gdk_pixbuf_new_from_file_at_scale(IMG_PEDAL_OFF, 60, 60, TRUE, NULL);
    gtk_image_set_from_pixbuf(pedalImg[0], pixbufPedalOff);
    gtk_image_set_from_pixbuf(pedalImg[1], pixbufPedalOff);

    timers[0].timerLabel = GTK_LABEL(gtk_builder_get_object(builder, "timer1"));
    timers[1].timerLabel = GTK_LABEL(gtk_builder_get_object(builder, "timer2"));
    timers[0].styleContext = gtk_widget_get_style_context(GTK_WIDGET(timers[0].timerLabel));
    timers[1].styleContext = gtk_widget_get_style_context(GTK_WIDGET(timers[1].timerLabel));
    timers[0].stopButton = GTK_WIDGET(gtk_builder_get_object(builder, "stopButton1"));
    timers[1].stopButton = GTK_WIDGET(gtk_builder_get_object(builder, "stopButton2"));

    name1 = GTK_LABEL(gtk_builder_get_object(builder, "name1"));
    name2 = GTK_LABEL(gtk_builder_get_object(builder, "name2"));
    reactionTime[0] = GTK_LABEL(gtk_builder_get_object(builder, "reactionTime1"));
    reactionTime[1] = GTK_LABEL(gtk_builder_get_object(builder, "reactionTime2"));
    runnerReadyLabel[0] = GTK_LABEL(gtk_builder_get_object(builder, "runnerReadyLabel1"));
    runnerReadyLabel[1] = GTK_LABEL(gtk_builder_get_object(builder, "runnerReadyLabel2"));
    startButton = GTK_BUTTON(gtk_builder_get_object(builder, "startButton"));
    startButton1 = GTK_BUTTON(gtk_builder_get_object(builder, "startButton1"));
    startButton2 = GTK_BUTTON(gtk_builder_get_object(builder, "startButton2"));

    // CSS
    //globalCssProvider = gtk_css_provider_new();
    cssStyleProvider = gtk_css_provider_new();
    technicalCssProvider = gtk_css_provider_new();
    //gtk_css_provider_load_from_path(globalCssProvider, CSS_GLOBAL_STYLE_PATH, NULL);
    gtk_css_provider_load_from_path(cssStyleProvider, CSS_STYLE_PATH, NULL);
    gtk_css_provider_load_from_path(technicalCssProvider, CSS_TECHNICAL_STYLE_PATH, NULL);
    //gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(globalCssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssStyleProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(technicalCssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Show
    gtk_widget_show_all(window);
    gtk_widget_hide(GTK_WIDGET(backButton));
    hideWidget(timers[0].stopButton);
    hideWidget(timers[1].stopButton);

    // Run app
    CHECK_T(
        pthread_create(&appThread, NULL, applicationThread, NULL),
        "--pthread_create()"
    );
}

// Functions de hide or show a widget without changing widget organization on screen
void hideWidget(GtkWidget * w) {
    #ifndef NO_STYLE_CHANGES
    gtk_style_context_add_class(gtk_widget_get_style_context(w), "hide");
    #endif
}
void showWidget(GtkWidget * w) {
    #ifndef NO_STYLE_CHANGES
    gtk_style_context_remove_class(gtk_widget_get_style_context(w), "hide");
    #endif
}

/* ------------------------------------------------------------------------------*/
/* APP THREAD */
/* ------------------------------------------------------------------------------*/

void * applicationThread() {
    char * data = NULL;
    char byte = 0;
    int count = 0;

    // Read data
    while (read(in, &byte, 1) == 1) {
        if (ioctl(in, FIONREAD, &count) != -1) {
            data = malloc(count + 1);
            data[0] = byte;
            if (read(in, data + 1, count) == count) {
                // Use data
                printf("DATA received : %s", data);
                analyseData(data);
            }
            free(data);
        }
    }

	pthread_exit(NULL);
}

// Functon called to parse data received from spidalBack
void analyseData(char * data) {
    // TODO - prendre en compte le cas où il y aurait plusieurs messages en même temps
    char * p1;
    int p2;
    long p3;
    long p4;

    p1 = strtok(data, ":");
    p2 = atoi(strtok(NULL, ":"));
    p3 = strtol(strtok(NULL, ":"), NULL, 10);
    p4 = strtol(strtok(NULL, ":"), NULL, 10);

    if (strcmp(p1, "REMOTE") == 0) remoteStatusChanged(p2);
    else if (strcmp(p1,"BTN1") == 0) btnStatusChanged(0,p2,p3,p4);
    else if (strcmp(p1,"BTN2") == 0) btnStatusChanged(1,p2,p3,p4);
    else if (strcmp(p1,"PEDAL1") == 0) pedalStatusChanged(0,p2,p3,p4);
    else if (strcmp(p1,"PEDAL2") == 0) pedalStatusChanged(1,p2,p3,p4);
}

/* ------------------------------------------------------------------------------*/
/* PHYSICAL STATUS CHANGES */
/* ------------------------------------------------------------------------------*/

// Function called when remote status has changed
void remoteStatusChanged(int status) {
    if (status) {
        gtk_image_set_from_pixbuf(networkIcon, pixbufNetworkIconOn);
        gtk_image_set_from_pixbuf(networkImgHome, pixbufNetworkImgHomeOn);
    } else {
        gtk_image_set_from_pixbuf(networkIcon, pixbufNetworkIconOff);
        gtk_image_set_from_pixbuf(networkImgHome, pixbufNetworkImgHomeOff);
    }
}

// Function called when a button status has changed
void btnStatusChanged(int n, int status, long t_s, long t_ns) {
    int s;
    int statusRuner;
    int statusOtherRuner;

    switch (getStatus(&appPage)) {
        case APP_MENU:
            if (status) gtk_image_set_from_pixbuf(btnImg[n], pixbufBtnOn);
            else gtk_image_set_from_pixbuf(btnImg[n], pixbufBtnOff);
        break;
        case APP_FREE_RACE:
            if (status && getStatus(&runnerStatus[n]) == RUNNER_RACE) {
                // End timer n
                setStatus(&runnerStatus[n], RUNNER_WAITING);
                stopTimer(&timers[n], t_s, t_ns);
                //printf("End free race %d\n", n);
            }
        break;
        case APP_TRAINING_DUO:
            s = getStatus(&raceStatus);
            if (s == RACE_ONGOING) {
                // Runner ended the race
                statusRuner = getStatus(&runnerStatus[n]);
                statusOtherRuner = getStatus(&runnerStatus[n == 0 ? 1 : 0]);
                if (statusRuner == RUNNER_RACE) {
                    setStatus(&runnerStatus[n], RUNNER_END_RACE);
                    stopTimer(&timers[n], t_s, t_ns);
                    if (statusOtherRuner == RUNNER_END_RACE) {
                        // End of race
                        endRace(1);
                    }
                }
            }
        break;
    }
}

// Function called when a pedal status has changed
void pedalStatusChanged(int n, int status, long t_s, long t_ns) {
    int s;
    int statusRuner;
    //int statusOtherRuner;

    switch (getStatus(&appPage)) {
        case APP_MENU:
            if (status) gtk_image_set_from_pixbuf(pedalImg[n], pixbufPedalOn);
            else gtk_image_set_from_pixbuf(pedalImg[n], pixbufPedalOff);
        break;
        case APP_FREE_RACE:
            if (!status && getStatus(&runnerStatus[n]) == RUNNER_WAITING) {
                // Start timer n
                setStatus(&runnerStatus[n], RUNNER_RACE);
                startTimer(&timers[n], t_s, t_ns);
                //printf("Start free race %d\n", n);
            }
        break;

        case APP_TRAINING_DUO:
            s = getStatus(&raceStatus);
            statusRuner = getStatus(&runnerStatus[n]);
            //statusOtherRuner = getStatus(&runnerStatus[n == 0 ? 1 : 0]);

            // Setup GUI 
            if (status) setRunnerReadyGUI(n, 1);
            else setRunnerReadyGUI(n, 0);

            if (status && s == RACE_WAITING) {
                // Pedal ON : Runner is ready
                setStatus(&runnerStatus[n], RUNNER_READY);
                //printf("Runner %d is ready\n", n);
            }
            else if (!status && s == RACE_WAITING) {
                // Pedal OFF : Runner is not ready
                setStatus(&runnerStatus[n], RUNNER_WAITING);
                #ifndef NO_STYLE_CHANGES
                gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(startButton)), "disabled");
                #endif
                //printf("Runner %d is not ready\n", n);
            }
            else if (!status && s == RACE_DECOUNT) {
                // Pedal OFF during race or decount
                // False start
                falseStart(n);
            }
            else if (!status && statusRuner == RUNNER_READY && s == RACE_ONGOING) {
                // Pedal OFF during race : runner started the race
                setStatus(&runnerStatus[n], RUNNER_RACE);
                // Saveing time for reaction time computation
                timers[n].reactionTimeStart.tv_sec = t_s;
                timers[n].reactionTimeStart.tv_nsec = t_ns;
                setReactionTime(n);
            }
            // else if (!status && statusRuner == RUNNER_READY && s == RACE_ONGOING) {
            //     // Pedal OFF during race or decount
            //     if (timeCompare(t_s, t_ns, timeRaceStart.tv_sec, timeRaceStart.tv_nsec) == -1) {
            //         // Pedal OFF during decount : FALSE START
            //         falseStart(n);
            //         //printf("False start for runner %d\n", n);
            //     }
            //     else {
            //         // Pedal OFF during race : runner started the race
            //         setStatus(&runnerStatus[n], RUNNER_RACE);
            //         // Saveing time for reaction time computation
            //         timers[n].reactionTimeStart.tv_sec = t_s;
            //         timers[n].reactionTimeStart.tv_nsec = t_ns;
            //         setReactionTime(n);
            //     }
            // }

            if (getStatus(&runnerStatus[0]) == RUNNER_READY && getStatus(&runnerStatus[1]) == RUNNER_READY) {
                // Race is ready
                #ifndef NO_STYLE_CHANGES
                gtk_style_context_remove_class(gtk_widget_get_style_context(GTK_WIDGET(startButton)), "disabled");
                #endif
            }
        break;
    }
}

/* ------------------------------------------------------------------------------*/
/* ON CLICK EVENTS */
/* ------------------------------------------------------------------------------*/

void on_backButton_clicked(GtkButton * b) {
    // Reset status
    setStatus(&appPage, APP_MENU);
    setStatus(&runnerStatus[0], RUNNER_WAITING);
    setStatus(&runnerStatus[1], RUNNER_WAITING);
    gtk_widget_hide(GTK_WIDGET(backButton));
    gtk_widget_show(GTK_WIDGET(shutDownButton));

    // Reset timers
    if (timerHandlerId != 0) {
        g_source_remove(timerHandlerId);
        timerHandlerId = 0;
    }
    timers[0].running = 0;
    timers[1].running = 0;
    hideWidget(timers[0].stopButton);
    hideWidget(timers[1].stopButton);

    gtk_header_bar_set_title(headerBar, "");
    
    // Change page
	gtk_stack_set_visible_child_name(stack, (const char *) "page0");
}

// Function called when Free race button is clicked on GUI
void on_freeRaceButton_clicked(GtkButton * b) {
    setStatus(&appPage, APP_FREE_RACE);

    gtk_header_bar_set_title(headerBar, "Course libre");
	gtk_label_set_text(name1, "Coureur 1");
	gtk_label_set_text(name2, "Coureur 2");
    gtk_widget_hide(GTK_WIDGET(reactionTime[0]));
    gtk_widget_hide(GTK_WIDGET(reactionTime[1]));
    hideWidget(GTK_WIDGET(runnerReadyLabel[0]));
    hideWidget(GTK_WIDGET(runnerReadyLabel[1]));
    gtk_widget_hide(GTK_WIDGET(startButton));
    gtk_widget_hide(GTK_WIDGET(startButton1));
    gtk_widget_hide(GTK_WIDGET(startButton2));

    gtk_widget_show(GTK_WIDGET(backButton));
    gtk_widget_hide(GTK_WIDGET(shutDownButton));

    cleanRaceGUI();

    // Change page
	gtk_stack_set_visible_child_name(stack, (const char *) "page1");
}

// Function called when training Solo Button is clicked on GUI
// void on_trainingSoloButton_clicked(GtkButton * b) {
//     setStatus(&appPage, APP_TRAINING_DUO);

//     gtk_header_bar_set_title(headerBar, "Entrainement Solo");
// 	gtk_label_set_text(name1, "Coureur 1");
// 	gtk_label_set_text(name2, "Coureur 2");
//     gtk_widget_show(GTK_WIDGET(reactionTime[0]));
//     gtk_widget_show(GTK_WIDGET(reactionTime[1]));
//     gtk_widget_hide(GTK_WIDGET(startButton));

//     gtk_widget_show(GTK_WIDGET(startButton1));
//     gtk_widget_show(GTK_WIDGET(startButton2));
//     gtk_widget_show(GTK_WIDGET(backButton));
//     gtk_widget_hide(GTK_WIDGET(shutDownButton));

//     cleanRaceGUI();

//     // Change page
// 	gtk_stack_set_visible_child_name(stack, (const char *) "page1");
// }

// Function called when training Duo Button is clicked on GUI
void on_trainingDuoButton_clicked(GtkButton * b) {
    setStatus(&appPage, APP_TRAINING_DUO);

    gtk_widget_show(GTK_WIDGET(startButton));
    gtk_widget_hide(GTK_WIDGET(startButton1));
    gtk_widget_hide(GTK_WIDGET(startButton2));

    gtk_widget_show(GTK_WIDGET(reactionTime[0]));
    gtk_widget_show(GTK_WIDGET(reactionTime[1]));

    gtk_header_bar_set_title(headerBar, "Entrainement Duo");
	gtk_label_set_text(name1, "Coureur 1");
	gtk_label_set_text(name2, "Coureur 2");

    gtk_widget_show(GTK_WIDGET(backButton));
    gtk_widget_hide(GTK_WIDGET(shutDownButton));

    initRace();

    // Change page
	gtk_stack_set_visible_child_name(stack, (const char *) "page1");
}

// Function called when strat button is clicked
void on_startButton_clicked(GtkButton * b) {
    // If the two runners are ready and app in the good state
    if (getStatus(&appPage) == APP_TRAINING_DUO
        && getStatus(&raceStatus) == RACE_WAITING
        && getStatus(&runnerStatus[0]) == RUNNER_READY
        && getStatus(&runnerStatus[1]) == RUNNER_READY) {
            // Start race decount
            startRace();
    } else if(getStatus(&appPage) == APP_TRAINING_DUO && getStatus(&raceStatus) == RACE_END) {
        // Init new race
        initRace();
    }
}

// Function called when stop button 1 is clicked
void on_stopButton1_clicked(GtkButton * b) {
    failRace(0);
}

// Function called when stop button 2 is clicked
void on_stopButton2_clicked(GtkButton * b) {
    failRace(1);
}

/* ------------------------------------------------------------------------------*/
/* TIMERS */
/* ------------------------------------------------------------------------------*/

// Function called to init timer appearence
void initTimer(timer_type * timer) {
	gtk_label_set_text(timer->timerLabel, "00:00:00");
    #ifndef NO_STYLE_CHANGES
    gtk_style_context_remove_class(timer->styleContext, "red");
    #endif
}

// Function called to set timer on error apearence
void setTimerOnError(timer_type * timer) {
    if (timer->running) {
        struct timespec timeInt;
        clock_gettime(CLOCK_REALTIME, &timeInt);
        stopTimer(timer, timeInt.tv_sec, timeInt.tv_nsec);
    }
    #ifndef NO_STYLE_CHANGES
    gtk_style_context_add_class(timer->styleContext, "red");
    #endif
}

// Function called to start a timer
void startTimer(timer_type * timer, long t_s, long t_ns) {
    initTimer(timer);
    timer->timeStart.tv_sec = t_s;
    timer->timeStart.tv_nsec = t_ns;
    timer->running = 1;

    showWidget(timer->stopButton);

    // Create timer handler if needed
    if (timerHandlerId == 0) timerHandlerId = g_timeout_add(GUI_TIMER_DELAY, (GSourceFunc) timerHandler, NULL);
}

// Function called to stop a timer
void stopTimer(timer_type * timer, long t_s, long t_ns) {
    char * string = malloc(9*sizeof(char));

    // Update timer with end value
    timer->timeEnd.tv_sec = t_s;
    timer->timeEnd.tv_nsec = t_ns;
    timer->running = 0;

    // Remove timer handler if needed
    if (!timers[0].running && !timers[1].running) {
        g_source_remove(timerHandlerId);
        timerHandlerId = 0;
    }

    // Compute timer interval
    computeTime(&(timer->t_m), &(timer->t_s), &(timer->t_ms), &(timer->timeStart), &(timer->timeEnd));

    // Print time interval
    sprintf(string, "%02d:%02d:%02d", timer->t_m, timer->t_s, timer->t_ms);
	gtk_label_set_text(timer->timerLabel, string);
	gtk_label_set_text(timer->timerLabel, string);
    free(string);

    hideWidget(timer->stopButton);
}

// Function to compute time interval of a timer
void computeTime(int * t_m, int * t_s, int * t_ms, struct timespec * timeI, struct timespec * timeF) {
    *t_m = 0;
    *t_s = timeF->tv_sec - timeI->tv_sec;
	*t_ms = timeF->tv_nsec - timeI->tv_nsec;
	if (*t_ms < 0) {
		*t_s = *t_s -1;
		*t_ms = (1000000000 + *t_ms);
	}
	*t_ms = *t_ms / 10000000;
    *t_m = *t_s / 60;
    *t_s = *t_s % 60;
}

// Function handling timer on GUI
gboolean timerHandler(void * p) {
    struct timespec timeInt;
	clock_gettime(CLOCK_REALTIME, &timeInt);
    char * string = malloc(9*sizeof(char));
    // Print on timer if needed
    for (int i = 0; i < 2; i++) {
        if (timers[i].running) {
            computeTime(&(timers[i].t_m), &(timers[i].t_s), &(timers[i].t_ms), &(timers[i].timeStart), &timeInt);
            if (timers[i].t_ms < 50) {
                sprintf(string, "%02d %02d %02d",timers[i].t_m, timers[i].t_s, timers[i].t_ms);
            } else {
                sprintf(string, "%02d:%02d:%02d",timers[i].t_m, timers[i].t_s, timers[i].t_ms);
            }
            gtk_label_set_text(timers[i].timerLabel, string);
        }
    }
    
	
    free(string);
	
	return TRUE;	
}

/**
 * if t1 < t2 return -1
 * if t1 > t2 return 1
 * if t1 == t2 return 0
 */
int timeCompare(long t1_s, long t1_ns, long t2_s, long t2_ns) {
    if ((t1_s < t2_s) || (t1_s == t2_s && t1_ns < t2_ns)) {
        return -1;
    } else if (t1_s == t2_s && t1_ns == t2_ns) {
        return 0;
    }
    return 1;
}

/**
 * if t1 < t2 return -1
 * if t1 > t2 return 1
 * if t1 == t2 return 0
 */
int timeCompareM(int t1_m, int t1_s, int t1_ms, int t2_m, int t2_s, int t2_ms) {
    if ((t1_m < t2_m) || (t1_m == t2_m && t1_s < t2_s) || (t1_m == t2_m && t1_s == t2_s && t1_ms < t2_ms)) {
        return -1;
    } else if (t1_m == t2_m && t1_s == t2_s && t1_ms == t2_ms) {
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------------------*/
/* RACE */
/* ------------------------------------------------------------------------------*/

void initRace() {
    #ifndef NO_STYLE_CHANGES
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(startButton)), "disabled");
    #endif
    gtk_button_set_label(startButton, "Démarrer");

    showWidget(GTK_WIDGET(runnerReadyLabel[0]));
    showWidget(GTK_WIDGET(runnerReadyLabel[1]));
    setRunnerReadyGUI(0, 0);
    setRunnerReadyGUI(1, 0);

    failedRace[0] = 0;
    failedRace[1] = 0;

    cleanRaceGUI();

    setStatus(&raceStatus, RACE_WAITING);
    setStatus(&runnerStatus[0], RUNNER_WAITING);
    setStatus(&runnerStatus[1], RUNNER_WAITING);
}

void cleanRaceGUI() {
    initTimer(&timers[0]);
    initTimer(&timers[1]);
    gtk_label_set_text(reactionTime[0], "Temps de réaction : - ms");
    gtk_label_set_text(reactionTime[1], "Temps de réaction : - ms");
    #ifndef NO_STYLE_CHANGES
    gtk_style_context_remove_class(timers[1].styleContext, "green");
    gtk_style_context_remove_class(timers[0].styleContext, "green");
    gtk_style_context_remove_class(timers[1].styleContext, "red");
    gtk_style_context_remove_class(timers[0].styleContext, "red");
    gtk_style_context_remove_class(gtk_widget_get_style_context(GTK_WIDGET(reactionTime[0])), "red");
    gtk_style_context_remove_class(gtk_widget_get_style_context(GTK_WIDGET(reactionTime[1])), "red");
    #endif
}

void startRace() {
    struct timespec time;
    struct itimerval timerAlarm;

    timerAlarm.it_value.tv_sec = STARTER_DURATION_SEC;
    timerAlarm.it_value.tv_usec = STARTER_DURATION_NSEC/1000;
    timerAlarm.it_interval.tv_sec = 0;
    timerAlarm.it_interval.tv_usec = 0;

    clock_gettime(CLOCK_REALTIME, &time);

    #ifndef NO_STYLE_CHANGES
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(startButton)), "disabled");
    #endif

    hideWidget(GTK_WIDGET(runnerReadyLabel[0]));
    hideWidget(GTK_WIDGET(runnerReadyLabel[1]));
    
    // timeRaceStart.tv_nsec = time.tv_nsec + STARTER_DURATION_NSEC;
    // timeRaceStart.tv_sec = time.tv_sec + STARTER_DURATION_SEC;
    // if (timeRaceStart.tv_nsec >= 1000000000) {
    //     timeRaceStart.tv_sec++;
    //     timeRaceStart.tv_nsec = timeRaceStart.tv_nsec - 1000000000;
    // }

    // Check if all runners are still ready before starting race
    if (getStatus(&runnerStatus[0]) == RUNNER_READY && getStatus(&runnerStatus[1]) == RUNNER_READY) {

        // Starting race
        setStatus(&raceStatus, RACE_DECOUNT);
        playStarter();

        // Wait until the end of the starter
        CHECK(setitimer(ITIMER_REAL, &timerAlarm, NULL), "setitimer()");
        // do {
        //     usleep(100);
        //     clock_gettime(CLOCK_REALTIME, &time);
        // } while (timeCompare(time.tv_sec, time.tv_nsec, timeRaceStart.tv_sec, timeRaceStart.tv_nsec) == -1 && getStatus(&raceStatus) == RACE_ONGOING);

        // // Check if there are no false start :
        // if (getStatus(&raceStatus) == RACE_ONGOING) {
        //     // Start timers
        //     startTimer(&timers[0], timeRaceStart.tv_sec, timeRaceStart.tv_nsec);
        //     startTimer(&timers[1], timeRaceStart.tv_sec, timeRaceStart.tv_nsec);
        // }
    }
}

void playStarter() {
    CHECK(soundProcess = fork(), "fork()");
    if (soundProcess == 0) {
        // Child process : Starter sound

        // Redirect output to /dev/null
        int fd = open("/dev/null", O_WRONLY);
        CHECK(dup2(fd, 1), "dup2()");
        CHECK(dup2(fd, 2), "dup2()");

        // Playing sound
        CHECK(execl(MPG123_PATH, "mpg123", COUNTDOWN_PATH, NULL), "execl()");

        // Should not be here
        exit(EXIT_FAILURE);
    }
}

void playFalseStartBuzzer() {
    CHECK(soundProcess = fork(), "fork()");
    if (soundProcess == 0) {
        // Child process : Starter sound

        // Redirect output to /dev/null
        int fd = open("/dev/null", O_WRONLY);
        CHECK(dup2(fd, 1), "dup2()");
        CHECK(dup2(fd, 2), "dup2()");

        // Playing sound
        CHECK(execl(MPG123_PATH, "mpg123", FALSE_START_BUZZER_PATH, NULL), "execl()");

        // Should not be here
        exit(EXIT_FAILURE);
    }
}

void endRace(int showVictory) {
    int timeComparison;
    setStatus(&raceStatus, RACE_END);
    #ifndef NO_STYLE_CHANGES
    gtk_style_context_remove_class(gtk_widget_get_style_context(GTK_WIDGET(startButton)), "disabled");
    #endif

    gtk_button_set_label(startButton, "Nouvelle course");
    #ifndef NO_STYLE_CHANGES
    if (showVictory) {
        if (!failedRace[0] && !failedRace[1]) {
            timeComparison = timeCompareM(timers[0].t_m, timers[0].t_s, timers[0].t_ms, timers[1].t_m, timers[1].t_s, timers[1].t_ms);
            if (timeComparison == -1) {
                // Runner 1 win
                gtk_style_context_add_class(timers[0].styleContext, "green");
                gtk_style_context_add_class(timers[1].styleContext, "red");
            } else if (timeComparison == 1) {
                // Runner 2 win
                gtk_style_context_add_class(timers[1].styleContext, "green");
                gtk_style_context_add_class(timers[0].styleContext, "red");
            } else {
                // Ex AEQUO
            }
        } else if (!failedRace[0]) {
            printf("Failed race 1\n");
            gtk_style_context_add_class(timers[0].styleContext, "green");
        } else if (!failedRace[1]) {
            gtk_style_context_add_class(timers[1].styleContext, "green");
        }
    }
    #endif
}

void failRace(int n) {
    if (getStatus(&runnerStatus[n]) == RUNNER_RACE) {
        // Stop race and set timer on error
        setTimerOnError(&timers[n]);
        setStatus(&runnerStatus[n], RUNNER_WAITING);

        if (getStatus(&appPage) == APP_TRAINING_DUO) {
            failedRace[n] = 1;
            setStatus(&runnerStatus[n], RUNNER_END_RACE);
            if (getStatus(&runnerStatus[n == 0 ? 1 : 0]) == RUNNER_END_RACE) {
                // If all runners failed
                endRace(0);
            }
        }
    }
}

void falseStart(int n) {
    kill(soundProcess, SIGKILL);
    playFalseStartBuzzer();
    setStatus(&raceStatus, RACE_END);
    #ifndef NO_STYLE_CHANGES
    gtk_style_context_add_class(timers[n].styleContext, "red");
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(reactionTime[n])), "red");
    #endif
    gtk_label_set_text(reactionTime[n], "Faux départ");
    endRace(0);
}

void setRunnerReadyGUI(int n, int ready) {
    if (ready) {
        gtk_label_set_text(runnerReadyLabel[n], "Prêt");
        #ifndef NO_STYLE_CHANGES
        gtk_style_context_remove_class(gtk_widget_get_style_context(GTK_WIDGET(runnerReadyLabel[n])), "red");
        gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(runnerReadyLabel[n])), "green");
        #endif
    } else {
        gtk_label_set_text(runnerReadyLabel[n], "Pas prêt");
        #ifndef NO_STYLE_CHANGES
        gtk_style_context_remove_class(gtk_widget_get_style_context(GTK_WIDGET(runnerReadyLabel[n])), "green");
        gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(runnerReadyLabel[n])), "red");
        #endif
    }
}

void setReactionTime(int n) {
    int t_m;
    int t_s;
    int t_ms;
    char * string = malloc(30*sizeof(char));
    computeTime(&t_m, &t_s, &t_ms, &(timers[n].timeStart) ,&(timers[n].reactionTimeStart));
    sprintf(string, "Temps de réaction : %d ms", t_m * 60000 + t_s * 1000 + t_ms);
    gtk_label_set_text(reactionTime[n], string);
    free(string);
}

void signalHandler(int sigNum) {
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    switch (sigNum) {
        case SIGALRM:
            // Race starting
            // Check if there are no false start :
            if (getStatus(&raceStatus) == RACE_DECOUNT) {
                // Start timers
                // startTimer(&timers[0], timeRaceStart.tv_sec, timeRaceStart.tv_nsec);
                // startTimer(&timers[1], timeRaceStart.tv_sec, timeRaceStart.tv_nsec);
                setStatus(&raceStatus, RACE_ONGOING);
                startTimer(&timers[0], time.tv_sec, time.tv_nsec);
                startTimer(&timers[1], time.tv_sec, time.tv_nsec);
            }
        break;
    }
}

void onWindowDestroy(GtkWidget * w) {
    exit(EXIT_SUCCESS);
}