cssdir    := static/css
cssfiles  := layout.css forms.css register.css
jsdir     := static/js
tmpldir   := template
tmplfiles := register.tmpl
scss      := $(shell which scss)
scssdir   := scss
scssflags := --no-cache
pug       := $(shell which pug)
pugdir    := pug
pugflags  := --extension tmpl --out $(tmpldir)
dev       := true

ifeq ($(dev), true)
scssflags += --style expanded
pugflags  += --pretty
else
scssflags += --style compressed --sourcemap=none
endif

vpath %.scss $(scssdir)
vpath %.css  $(cssdir)
vpath %.pug  $(pugdir)
vpath %.tmpl $(tmpldir)

.PHONY: run
run: all
	bower install
	go build github.com/coldume/pulse/cmd/server
	./server

.PHONY: all
all: css tmpl

.PHONY: css
css: $(cssfiles)

%.css: %.scss | $(cssdir)
	$(scss) $^ $(cssdir)/$@ $(scssflags)

$(cssdir):
	mkdir -p $@

.PHONY: tmpl
tmpl: $(tmplfiles)

%.tmpl: %.pug | $(tmpldir)
	$(pug) $^ $(pugflags)

$(tmpldir):
	mkdir -p $@

.PHONY: clean
clean:
	rm -Rf static template server
