/*
 * Copyright (C) 2010 Das Android Open Source-Projekt
 *
 * Lizenziert unter der Apache-Lizenz, Version 2.0 ("Lizenz");
 * Sie dürfen diese Datei nur gemäß den Lizenzbedingungen verwenden.
 * Eine Kopie der Lizenz erhalten Sie unter:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Sofern nicht durch geltendes Recht anders festgelegt oder schriftlich vereinbart, wird
 * die unter der Lizenz vertriebene Software WIE BESEHEN,
 * OHNE GARANTIEN ODER BEDINGUNGEN GLEICH WELCHER ART, seien sie ausdrücklich oder konkludent, zur Verfügung gestellt.
 * Die unter der Lizenz geltenden Berechtigungen und Einschränkungen entnehmen Sie
 * der Lizenz in der jeweiligen Sprache.
 *
 */

#ifndef _ANDROID_NATIVE_APP_GLUE_H
#define _ANDROID_NATIVE_APP_GLUE_H

#include <poll.h>
#include <pthread.h>
#include <sched.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Die von <android/native_activity.h> bereitgestellte Schnittstelle für systemeigene Aktivitäten
 * basiert auf einer Reihe von von Anwendungen bereitgestellten Rückrufen, die 
 * vom Hauptthread der Anwendung aufgerufen werden, wenn bestimmte Ereignisse eintreten.
 *
 * Das bedeutet, dass keiner dieser Rückrufe _blockieren_ _darf_, andernfalls
 * besteht die Gefahr, dass das System das Schließen der Anwendung erzwingt. Dieses
 * Programmiermodell ist direkt, unaufwändig, weist aber Einschränkungen auf.
 *
 * Mithilfe der statischen Bibliothek "threaded_native_app" kann ein anderes
 * Ausführungsmodell bereitgestellt werden, bei dem die Anwendung ihre eigene Hauptereignisschleife
 * in einem anderen Thread implementieren kann. Das funktioniert so:
 *
 * 1/ Die Anwendung muss eine Funktion mit dem Namen "android_main()" bereitstellen,
 *    die beim Erstellen der Aktivität in einem neuen Thread, der sich
 *    vom Hauptthread der Aktivität unterscheidet, aufgerufen wird.
 *
 * 2/ android_main() empfängt einen Zeiger auf eine gültige "android_app"-Struktur,
 *    die Verweise auf andere wichtige Objekte enthält, z. B. die 
 *    ANativeActivity-Objektinstanz, in der die Anwendung ausgeführt wird.
 *
 * 3/ Das Objekt "android_app" enthält eine ALooper-Instanz, die bereits auf zwei wichtige
 *    Dinge lauscht:
 *
 *      - Lebenszyklusereignisse der Aktivität (z. B. "Anhalten", "Fortsetzen"). Siehe dazu unten die APP_CMD_XXX-
 *        Deklarationen.
 *
 *      - Eingabeereignisse, die aus der an die Aktivität angefügte AInputQueue kommen.
 *
 *    Jede davon entsprechen einem ALooper-Bezeichner, der von
 *    ALooper_pollOnce mit den Werten LOOPER_ID_MAIN bzw. LOOPER_ID_INPUT
 *    zurückgegeben wird.
 *
 *    Ihre Anwendung kann den gleichen ALooper verwenden, um auf weitere
 *    Dateideskriptoren zu lauschen. Sie können entweder rückrufbasiert sein oder mit
 *    Rückgabebezeichnern, die mit LOOPER_ID_USER beginnen.
 *
 * 4/ Immer, wenn Sie ein LOOPER_ID_MAIN- oder LOOPER_ID_INPUT-Ereignis empfangen,
 *    zeigen die zurückgegebenen Daten auf eine android_poll_source-Struktur. Sie können
 *    dafür die Funktion "process()" aufrufen und "android_app->onAppCmd"
 *    sowie "android_app->onInputEvent" einsetzen, die dann für Ihre eigene Verarbeitung
 *    des Ereignisses aufgerufen werden.
 *
 *    Alternativ können Sie die Funktionen auf niederer Ebene aufrufen, um die
 *    Daten direkt zu verarbeiten... Beispiele für die Umsetzung finden Sie in den Implementierungen von
 *    "process_cmd()" und "process_input()" im Glue Code.
 *
 * Ein vollständiges Syntaxbeispiel finden Sie im Beispiel mit dem Namen "native-activity",
 * das im NDK enthalten ist. Weitere Informationen finden Sie auch im JavaDoc von " NativeActivity".
 */

struct android_app;

/**
 * Einem ALooper fd zugeordnete Daten, die als die "outData" zurückgegeben
 * werden, wenn in der Quelle Daten bereit sind.
 */
struct android_poll_source {
    // Der Bezeichner dieser Quelle.  Das kann LOOPER_ID_MAIN oder
    // LOOPER_ID_INPUT sein.
    int32_t id;

    // Die android_app, der dieser ident zugeordnet ist.
    struct android_app* app;

    // Aufzurufende Funktion zum Durchführen der Standardverarbeitung von Daten aus
    // dieser Quelle.
    void (*process)(struct android_app* app, struct android_poll_source* source);
};

/**
 * Dies ist die Schnittstelle für den standardmäßigen Verbindungscode einer
 * Threadanwendung. In diesem Modell wird der Code der Anwendung in einem
 * eigenen Thread getrennt vom Hauptthread des Prozesses ausgeführt.
 * Es ist nicht erforderlich, dass dieser Thread der Java
 * VM zugeordnet ist, obwohl das für JNI-Aufrufe von
 * Java-Objekten erforderlich ist.
 */
struct android_app {
    // Die Anwendung kann hier einen Zeiger auf ihr eigenes
    // Statusobjekt platzieren, wenn das erwünscht ist.
    void* userData;

    // Setzen Sie hier die Funktion zum Verarbeiten von Befehlen der Haupt-App ein (APP_CMD_*)
    void (*onAppCmd)(struct android_app* app, int32_t cmd);

    // Setzen Sie hier die Funktion zum Verarbeiten von Eingabeereignissen ein. An diesem Punkt
    // wurde die Zuteilung des Ereignisses bereits vorbereitet und wird bei der
    // Rückgabe abgeschlossen. Geben Sie 1 für verarbeitete Ereignisse, 0 für standardmäßige
    // Zuteilung zurück.
    int32_t (*onInputEvent)(struct android_app* app, AInputEvent* event);

    // Die ANativeActivity-Objektinstanz, in der diese App ausgeführt wird.
    ANativeActivity* activity;

    // Die aktuelle Konfiguration, in der die App ausgeführt wird.
    AConfiguration* config;

    // Die ist der gespeicherte Status der letzten Instanz, wie der zum Erstellungszeitpunkt übergeben wird.
    // Er lautet NULL, wenn es keinen Status gab. Sie können dies bei Bedarf verwenden; der
    // Arbeitsspeicher bleibt erhalten, bis Sie "android_app_exec_cmd()" für
    // APP_CMD_RESUME aufrufen, wodurch er freigegeben und "savedState" auf NULL gesetzt wird.
    // Diese Variablen sollten nur bei der Verarbeitung eines APP_CMD_SAVE_STATE geändert
    // werden und werden an diesem Punkt mit NULL initialisiert. Sie können dann einen malloc
    // des Status ausführen und die Informationen hier einsetzen. In diesem Fall wird der
    // Arbeitsspeicher später für Sie freigegeben.
    void* savedState;
    size_t savedStateSize;

    // Der dem Thread der App zugeordnete ALooper.
    ALooper* looper;

    // Bei einem anderen Wert als NULL ist dies die Eingabewarteschlange, aus der die App
    // Benutzereingabeereignisse empfängt.
    AInputQueue* inputQueue;

    // Bei einem anderen Wert als NULL ist dies die Fensteroberfläche, auf der die App zeichnen kann.
    ANativeWindow* window;

    // Aktuelles Inhaltsrechteck des Fensters; dies ist der Bereich in dem der
    // Inhalt des Fensters platziert werden soll, damit der Benutzer ihn sehen kann.
    ARect contentRect;

    // Aktueller Status der Aktivität der App. Kann entweder APP_CMD_START,
    // APP_CMD_RESUME, APP_CMD_PAUSE oder APP_CMD_STOP sein; weitere Info unten.
    int activityState;

    // Dieser Wert ist nicht Null, wenn die NativeActivity der Anwendung
    // entfernt und auf den Abschluss des Threads der App gewartet wird.
    int destroyRequested;

    // -------------------------------------------------
    // Unten folgen "private" Implementierungen des Glue Codes.

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    int msgread;
    int msgwrite;

    pthread_t thread;

    struct android_poll_source cmdPollSource;
    struct android_poll_source inputPollSource;

    int running;
    int stateSaved;
    int destroyed;
    int redrawNeeded;
    AInputQueue* pendingInputQueue;
    ANativeWindow* pendingWindow;
    ARect pendingContentRect;
};

enum {
    /**
     * Looper-Daten-ID von Befehlen, die aus dem Hauptthread der App stammen, der
     * als Bezeichner von "ALooper_pollOnce()" zurückgegeben wird. Die Daten für diesen
     * Bezeichner sind ein Zeiger auf eine android_poll_source-Struktur.
     * Diese können mit android_app_read_cmd() und android_app_exec_cmd()
     * abgerufen und bearbeitet werden.
     */
    LOOPER_ID_MAIN = 1,

    /**
     * Looper-Daten-ID von Ereignissen, die aus der AInputQueue des
     * Fensters der Anwendung kommen, das als Bezeichner von
     * ALooper_pollOnce() zurückgegeben wird. Die Daten für diesen Bezeichner sind ein Zeiger auf eine
     * android_poll_source-Struktur. Sie können über das inputQueue-Objekt
     * von android_app gelesen werden.
     */
    LOOPER_ID_INPUT = 2,

    /**
     * Start der benutzerdefinierten ALooper-Bezeichner.
     */
    LOOPER_ID_USER = 3,
};

enum {
    /**
     * Befehl aus dem Hauptthread: die AInputQueue hat sich geändert. Bei der Verarbeitung
     * dieses Befehls wird android_app->inputQueue auf die neue Warteschlange
     * (oder NULL) aktualisiert.
     */
    APP_CMD_INPUT_CHANGED,

    /**
     * Befehl vom Hauptthread: Ein neues ANativeWindow ist bereit zur Nutzung. Beim
     * Empfang dieses Befehls enthält android_app->window die neue
     * Fensterfläche.
     */
    APP_CMD_INIT_WINDOW,

    /**
     * Befehl vom Hauptthread: das vorhandene ANativeWindow muss beendet
     * werden. Beim Empfang dieses Befehls enthält android_app->window noch das
     * vorhandene Fenster und wird nach dem Aufruf von android_app_exec_cmd
     * auf NULL gesetzt.
     */
    APP_CMD_TERM_WINDOW,

    /**
     * Befehl vom Hauptthread: die Größe des aktuellen ANativeWindow wurde geändert.
     * Bitte mit der neuen Größe neu zeichnen.
     */
    APP_CMD_WINDOW_RESIZED,

    /**
     * Befehl vom Hauptthread: das System muss das aktuelle ANativeWindow neu
     * zeichnen lassen. Sie sollten das Fenster neu zeichnen, bevor Sie dieses an
     * android_app_exec_cmd() übergeben, um vorübergehende Darstellungsfehler zu vermeiden.
     */
    APP_CMD_WINDOW_REDRAW_NEEDED,

    /**
     * Befehl vom Hauptthread: Der Inhaltsbereich des Fensters hat sich geändert,
     * etwa durch Ein- oder Ausblenden des Bildschirm-Eingabefensters. Sie können
     * das neue Inhaltsrechteck in android_app::contentRect finden.
     */
    APP_CMD_CONTENT_RECT_CHANGED,

    /**
     * Befehl vom Hauptthread: Das Aktivitätsfenster der App hat den
     * Eingabefokus erhalten.
     */
    APP_CMD_GAINED_FOCUS,

    /**
     * Befehl vom Hauptthread: Das Aktivitätsfenster der App hat den
     * Eingabefokus verloren.
     */
    APP_CMD_LOST_FOCUS,

    /**
     * Befehl vom Hauptthread: Die aktuelle Gerätekonfiguration hat sich geändert.
     */
    APP_CMD_CONFIG_CHANGED,

    /**
     * Befehl vom Hauptthread: Das System hat wenig Arbeitsspeicher.
     * Versuchen Sie, die Speicherauslastung zu verringern.
     */
    APP_CMD_LOW_MEMORY,

    /**
     * Befehl vom Hauptthread: Die Aktivität der App wurde gestartet.
     */
    APP_CMD_START,

    /**
     * Befehl vom Hauptthread: Die Aktivität der App wurde wieder aufgenommen.
     */
    APP_CMD_RESUME,

    /**
     * Befehl vom Hauptthread: Die App sollte einen neuen gespeicherten Status
     * für sich generieren, um ihn bei Bedarf wiederherstellen zu können. Wenn Sie über einen gespeicherten Status verfügen,
     * weisen Sie ihn mit malloc zu, und platzieren Sie ihn in android_app.savedState mit
     * der Größe in android_app.savedStateSize. Sie werden später für Sie
     * freigegeben.
     */
    APP_CMD_SAVE_STATE,

    /**
     * Befehl vom Hauptthread: die Aktivität der App wurde angehalten.
     */
    APP_CMD_PAUSE,

    /**
     * Befehl vom Hauptthread: Die Aktivität der App wurde beendet.
     */
    APP_CMD_STOP,

    /**
     * Befehl vom Hauptthread: Die Aktivität der App wird entfernt, und vor
     * dem Fortfahren wird auf das Aufräumen und Schließen des Hauptthreads der App gewartet.
     */
    APP_CMD_DESTROY,
};

/**
 * Aufrufen, wenn ALooper_pollAll() LOOPER_ID_MAIN zurückgibt, Lesen der
 * nächsten App-Befehlsnachricht.
 */
int8_t android_app_read_cmd(struct android_app* android_app);

/**
 * Aufrufen mit dem von android_app_read_cmd() zurückgegebenen Befehl, um
 * die anfängliche Vorverarbeitung des angegebenen Befehls auszuführen. Sie können nach dem Aufrufen dieser Funktion
 * eigene Aktionen für den Befehl ausführen.
 */
void android_app_pre_exec_cmd(struct android_app* android_app, int8_t cmd);

/**
 * Aufrufen mit dem von android_app_read_cmd() zurückgegebenen Befehl, um die
 * abschließende Nachverarbeitung des angegebenen Befehls auszuführen. Sie müssen Ihre eigenen Aktionen für den
 * Befehl vor dem Aufrufen dieser Funktion ausgeführt haben.
 */
void android_app_post_exec_cmd(struct android_app* android_app, int8_t cmd);

/**
 * Dies ist die Funktion, die der Anwendungscode implementieren muss, er
 * stellt den Haupteinsprungspunkt der App dar.
 */
extern void android_main(struct android_app* app);

#ifdef __cplusplus
}
#endif

#endif /* _ANDROID_NATIVE_APP_GLUE_H */
