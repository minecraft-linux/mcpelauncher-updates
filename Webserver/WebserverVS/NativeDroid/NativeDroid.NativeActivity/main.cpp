/*
* Copyright (C) 2010 Das Android Open Source-Projekt
 *
 * Lizenziert unter der Apache-Lizenz, Version 2.0 (der "Lizenz");
* Sie dürfen diese Datei nur gemäß den Bedingungen der Lizenz verwenden.
 * Sie können eine Kopie der Lizenz unter
*
*      http://www.apache.org/licenses/LICENSE-2.0 erhalten
*
* Sofern nicht durch geltendes Recht oder durch schriftliche Zustimmung anders festgelegt, wird
 * die unter der Lizenz vertriebene Software "WIE BESEHEN",
 * OHNE GARANTIEN ODER BEDINGUNGEN GLEICH WELCHER ART, seien sie ausdrücklich oder konkludent, zur Verfügung gestellt.
 * Die unter der Lizenz geltenden Berechtigungen und Einschränkungen finden Sie
* in der Lizenz für die bestimmte Sprache.
* 
*/

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

/**
* Unsere gespeicherten Statusdaten.
*/
struct saved_state {
	float angle;
	int32_t x;
	int32_t y;
};

/**
* Freigegebener Status für unsere App.
*/
struct engine {
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
	struct saved_state state;
};

/**
* Initialisieren eines EGL-Kontexts für die aktuelle Anzeige.
*/
static int engine_init_display(struct engine* engine) {
	// OpenGL-ES und -EGL initialisieren

	/*
	* Geben Sie hier die Attribute der gewünschten Konfiguration an.
	* Unten wählen wir eine EGLConfig mit mindestens 8 Bits pro
	* Farbkomponente aus, die mit den Fenstern auf dem Bildschirm kompatibel ist
	*/
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Hier wählt die Anwendung die Konfiguration, die sie wünscht. In diesem
	* Beispiel haben wir einen stark vereinfachten Auswahlprozess, in dem wir die 
	* erste EGLConfig auswählen, die unseren Kriterien entspricht. */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* "EGL_NATIVE_VISUAL_ID" ist ein Attribut der "EGLConfig", dessen
	* Akzeptierung durch "ANativeWindow_setBuffersGeometry()" garantiert ist.
	* Sobald wir eine "EGLConfig" ausgewählt haben, können wir die ANativeWindow-Puffer
	* gefahrlos mithilfe von  "EGL_NATIVE_VISUAL_ID" entsprechend neu konfigurieren. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;
	engine->state.angle = 0;

	// GL-Status initialisieren.
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	return 0;
}

/**
* Nur das aktuelle Bild in der Anzeige.
*/
static void engine_draw_frame(struct engine* engine) {
	if (engine->display == NULL) {
		// Keine Anzeige.
		return;
	}

	// Einfach den Bildschirm mit einer Farbe füllen.
	glClearColor(((float)engine->state.x) / engine->width, engine->state.angle,
		((float)engine->state.y) / engine->height, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(engine->display, engine->surface);
}

/**
* Den aktuell der Anzeige zugeordneten EGL-Kontext entfernen.
*/
static void engine_term_display(struct engine* engine) {
	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->animating = 0;
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
}

/**
* Das nächste Eingabeereignis verarbeiten.
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		engine->state.x = AMotionEvent_getX(event, 0);
		engine->state.y = AMotionEvent_getY(event, 0);
		return 1;
	}
	else if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
	{
		uint32_t code = AKeyEvent_getKeyCode(event);
	}
	return 0;
}

/**
* Den nächsten Hauptbefehl verarbeiten.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// Das System hat uns aufgefordert, unseren aktuellen Status zu speichern. Tun Sie das.
		engine->app->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->app->savedState) = engine->state;
		engine->app->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// Das Fenster wird angezeigt, stellen Sie es fertig.
		if (engine->app->window != NULL) {
			engine_init_display(engine);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// Das Fenster wird ausgeblendet oder geschlossen, bereinigen Sie es.
		engine_term_display(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		// Wenn unsere App den Fokus erhält, beginnen wir mit der Überwachung des Beschleunigungsmessers.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
			// Wir möchten 60 Ereignisse pro Sekunde empfangen.
			ASensorEventQueue_setEventRate(engine->sensorEventQueue,
				engine->accelerometerSensor, (1000L / 60) * 1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// Wenn unsere App den Fokus verliert, beenden wir die Überwachung des Beschleunigungsmessers.
		// Das dient dazu, die Batterie zu schonen, während die App nicht verwendet wird.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
		}
		// Auch keine Animationen mehr.
		engine->animating = 0;
		engine_draw_frame(engine);
		break;
	}
}

/**
* Dies ist der Haupteinsprungspunkt einer systemeigenen Anwendung, die 
* android_native_app_glue verwendet. Sie wird im eigenen Thread ausgeführt, mit ihrer eigenen
* Ereignisschleife zum Empfangen von Eingabeereignissen und Ausführen anderer Dinge.
*/
void android_main(struct android_app* state) {
	struct engine engine;

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

	// Auf Überwachung des Beschleunigungsmessers vorbereiten
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
		ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
		state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL) {
		// Wir beginnen mit einem zuvor gespeicherten Status; stellen Sie die App aus diesem wieder her.
		engine.state = *(struct saved_state*)state->savedState;
	}

	engine.animating = 1;

	// Schleife, die auf etwas zu tun wartet.

	while (1) {
		// Alle ausstehenden Ereignisse lesen.
		int ident;
		int events;
		struct android_poll_source* source;

		// Wenn wir nicht animieren, blockieren wir das Warten auf Ereignisse dauerhaft.
		// Wenn wir animieren, wird die Schleife ausgeführt, bis alle Ereignisse gelesen wurden, dann fahren
		// wir fort, das nächste Bild der Animation zu zeichnen.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
			(void**)&source)) >= 0) {

			// Dieses Ereignis verarbeiten.
			if (source != NULL) {
				source->process(state, source);
			}

			// Wenn ein Sensor Daten hat, diese jetzt verarbeiten.
			if (ident == LOOPER_ID_USER) {
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
						&event, 1) > 0) {
						LOGI("accelerometer: x=%f y=%f z=%f",
							event.acceleration.x, event.acceleration.y,
							event.acceleration.z);
					}
				}
			}

			// Überprüfen, ob wir beenden.
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}

		if (engine.animating) {
			// Fertig mit Ereignissen, nächstes Animationsbild zeichnen.
			engine.state.angle += .01f;
			if (engine.state.angle > 1) {
				engine.state.angle = 0;
			}

			// Das Zeichnen wird auf die Aktualisierungsrate des Bildschirms gedrosselt,
			// daher muss hier kein Timing erfolgen.
			engine_draw_frame(&engine);
		}
	}
}
