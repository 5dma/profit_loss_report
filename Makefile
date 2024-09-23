CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` `pkg-config --cflags json-glib-1.0`
LFLAGS = -lm `pkg-config --libs gtk+-3.0` `pkg-config --libs json-glib-1.0` `pkg-config --libs sqlite3`
OBJFILES = accounts_files.o accumulate.o add_account_control.o app_activate.o cleanup.o delete_account_control.o display_settings_control.o initialize.o main.o reports.o settings_view.o view.o

all: profit_loss_report

profit_loss_report: $(OBJFILES)
	$(CC) -o $@ $^ $(LFLAGS)

accounts_files.o: accounts_files.c
	$(CC) -c $< $(CFLAGS)

accumulate.o: accumulate.c
	$(CC) -c $< $(CFLAGS)

add_account_control.o: add_account_control.c
	$(CC) -c $< $(CFLAGS)

app_activate.o: app_activate.c
	$(CC) -c $< $(CFLAGS)

cleanup.o: cleanup.c
	$(CC) -c $< $(CFLAGS)

delete_account_control.o: delete_account_control.c
	$(CC) -c $< $(CFLAGS)

display_settings_control.o: display_settings_control.c
	$(CC) -c $< $(CFLAGS)

initialize.o: initialize.c
	$(CC) -c $< $(CFLAGS)

main.o: main.c
	$(CC) -c $< $(CFLAGS)

reports.o: reports.c
	$(CC) -c $< $(CFLAGS)

settings_view.o: settings_view.c
	$(CC) -c $< $(CFLAGS)

view.o: view.c
	$(CC) -c $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJFILES)

