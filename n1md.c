/*
 * new etar125's markdown to html
 * Copyright (c) 2026 etar125 Admanse
 * Licensed under ISC license (see LICENSE)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <e1l.h>
#include "estrdup.h"

#define llen(x) sizeof(x)/sizeof(x[0])

typedef int (*parser)(const char *begin, const char *end, int newblock);

typedef struct {
    char *search;
    int process;
    char *begin;
    char *end;
    size_t searchlen;
} st;

typedef struct {
    char *old;
    char *new;
    size_t oldlen;
} rp;

static void printh(const char *begin, const char *end);

static int doprefix(const char *begin, const char *end, int newblock);
static int dosurround(const char *begin, const char *end, int newblock);
static int doparagraph(const char *begin, const char *end, int newblock);
static int doreplace(const char *begin, const char *end, int newblock);
static int donewline(const char *begin, const char *end, int newblock);

static void process(const char *begin, const char *end, int newblock);

static parser ps[] = { doprefix, doparagraph, donewline, dosurround, doreplace };

static rp replace[] = {
    { "\\\\",   "\\" },
    { "\\*",    "*" },
    { "\\`",    "`" },
    { "\\#",    "#" },
    
    { "&amp;", "&amp;" },
    { "&",     "&amp;" },
    { ">",     "&gt;" },
    { "<",     "&lt;" },
    { "\"",    "&quot;" }
};

static st prefix[] = {
    /* 
     * 0 - hprint
     * 1 - collect and hprint
     * 2 - collect and process as newblock
     */
    { "# ", 0, "<h1>", "</h1>" },
    { "## ", 0, "<h2>", "</h2>" },
    { "### ", 0, "<h3>", "</h3>" },
    { "#### ", 0, "<h4>", "</h4>" },
    { "##### ", 0, "<h5>", "</h5>" },
    { "###### ", 0, "<h6>", "</h6>" },
    { ">", 2, "<blockquote>", "</blockquote>" },
    { "    ", 1, "<pre><code>", "</code></pre>" },
    { "\t", 1, "<pre><code>", "</code></pre>" },
};

static st surround[] = {
    { "`", 0, "<code>", "</code>" },
    { "***", 1, "<b><i>", "</i></b>" },
    { "**", 1, "<b>", "</b>" },
    { "*", 1, "<i>", "</i>" },
};

void printh(const char *begin, const char *end) {
    const char *p;

    for (p = begin; p < end; p++) {
        if (*p == '&')
            printf("&amp;");
        else if (*p == '>')
            printf("&gt;");
        else if (*p == '<')
            printf("&lt;");
        else if (*p == '"')
            printf("&quot;");
        else
            putc(*p, stdout);
    }
}

int doprefix(const char *begin, const char *end, int newblock) {
    const char *p, *q, *stop;
    char *ds, *rs;
    int i, c;
    size_t l, asize, bsize;

    p = begin;
    c = 0;

    for (i = 0; i < llen(prefix); i++) {
        l = prefix[i].searchlen;
        if (end - begin < l || strncmp(begin, prefix[i].search, l) != 0)
            continue;
        if (prefix[i].process == 0) {
            p += l;
            for (q = p; q < end && *q != '\n'; q++);
            if (q == p)
                return 0;
            printf("%s", prefix[i].begin);
            if (prefix[i].process)
                process(p, q, 0);
            else
                printh(p, q);
            puts(prefix[i].end);
        } else {
            ds = NULL, asize = 0, bsize = 0;
            stop = strstr(begin, "\n\n");
            if (!stop || stop > end)
                stop = end;

            if (prefix[i].search[0] == '>')
                c = 1;

            for (; p < stop; p++) {
                if (strncmp(p, prefix[i].search, l) != 0) {
                    stop = p;
                    if (*p == '\n')
                        p--;
                    break;
                }
                p += l;
                if (c && p < stop && *p == ' ')
                    p++;
                for (q = p; q < stop && *q != '\n'; q++);
                if (q == p)
                    continue;
                for (; p <= q; p++) {
                    if (d_append(&ds, &asize, &bsize, p, 1) != 0) {
                        if (ds)
                            free(ds);
                        perror("d_append failed");
                        return 0;
                    }
                }
                p = q;
            }

            rs = d_shrink(ds, asize, bsize);
            free(ds);
            if (!rs) {
                perror("d_shrink failed");
                return 0;
            }

            printf("%s", prefix[i].begin);
            if (prefix[i].process == 2)
                process(rs, rs + asize, 1);
            else
                printh(rs, rs + asize);
            puts(prefix[i].end);
        }
        return -(q - begin);
    }

    return 0;
}

int dosurround(const char *begin, const char *end, int newblock) {
    const char *p, *q;
    int i;
    size_t l;
    
    p = begin;

    for (i = 0; i < llen(surround); i++) {
        l = surround[i].searchlen;
        if (end - begin < l * 2 || strncmp(begin, surround[i].search, l) != 0)
            continue;
        p += l;
        for (q = p; q + l <= end && strncmp(q, surround[i].search, l) != 0; q++);
        if (q + l > end)
            return 0;
        printf("%s", surround[i].begin);
        if (surround[i].process)
            process(p, q, 0);
        else
            printh(p, q);
        printf("%s", surround[i].end);
        return q - begin + l;
    }

    return 0;
}

int doparagraph(const char *begin, const char *end, int newblock) {
    const char *p, *q;

    if (!newblock)
        return 0;

    p = strstr(begin, "\n\n");
    if (!p || p > end)
        p = end;
    q = p;
    for (; p > begin && *(p - 1) == '\n'; p--);
    if (p == begin) { return 0; }

    printf("<p>");
    process(begin, p, 0);
    puts("</p>");

    return -(q - begin);
}

int doreplace(const char *begin, const char *end, int newblock) {
    const char *p;
    int i;
    size_t l;

    p = begin;

    for (i = 0; i < llen(replace); i++) {
        l = replace[i].oldlen;
        if (p + l > end || strncmp(p, replace[i].old, l) != 0)
            continue;
        printf("%s", replace[i].new);
        return l;
    }

    return 0;
}

int donewline(const char *begin, const char *end, int newblock) {
    const char *p;

    for (p = begin; p < end && *p != '\n'; p++);
    
    p -= 2;

    if (p < begin || strncmp(p, "  ", 2) != 0)
        return 0;

    if (p + 3 >= end || p[3] == '\n') {
        process(begin, p, 0);
        return p - begin + 4;
    } else {
        process(begin, p, 0);
        puts("<br>");
        return p - begin + 3;
    }
}

void process(const char *begin, const char *end, int newblock) {
    int affected;
    const char *p, *q;
    int i;

    for (p = begin; p < end;) {
        if (newblock)
            while(*p == '\n')
                if (++p == end)
                    return;
        affected = 0;
        for (i = 0; i < llen(ps) && !affected; i++)
            affected = ps[i](p, end, newblock);
        p += abs(affected);
        if (!affected) {
            printh(p, p + 1);
            p++;
        }
        for (q = p; q < end && *q == '\n'; q++);
        if (q == end)
            return;
        if (p[0] == '\n' && p + 1 != end && p[1] == '\n')
            newblock = 1;
        else
            newblock = affected < 0;
    }
}

char *progname;

static int usage() {
    printf("n1md v"VERSION" by etar125 Admanse\n"
           "Usage: %s [file.md]\n"
           "Examples:\n"
           "    cat file.md | %s > file.html\n"
           "    %s file.md > file.html\n",
           progname, progname, progname);
    return 0;
}

int main(int argc, char **argv) {
    FILE *f = stdin;
    int i, fi = -1;
    char *fname = NULL, *buf = NULL, *ds = NULL, ch;
    size_t fz, asize = 0, bsize = 0;

    progname = argv[0];

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 ||
            strcmp(argv[i], "-v") == 0) {
            return usage();
        } else {
            fi = i;
        }
    }

    if (fi != -1) {
        fname = argv[fi];
        f = NULL;
        f = fopen(fname, "r");
        if (!f) {
            perror("fopen");
            return 1;
        }
        fseek(f, 0, SEEK_END);
        fz = ftell(f);
        rewind(f);

        buf = malloc(fz + 1);
        if (!buf) {
            perror("malloc");
            return 1;
        }

        fread(buf, 1, fz, f);
        fclose(f);
        buf[fz] = '\0';
    } else {
        while ((ch = fgetc(f)) != EOF) {
            if (d_append(&ds, &asize, &bsize, &ch, 1) != 0) {
                if (ds) { free(ds); }
                return 1;
            }
        }
        buf = d_shrink(ds, asize, bsize);
        if (ds) { free(ds); }
        if (!buf) { return 1; }
        fz = asize;
    }

    /* Вычисление размеров */

    for (i = 0; i < llen(prefix); i++)
        prefix[i].searchlen = strlen(prefix[i].search);

    for (i = 0; i < llen(surround); i++)
        surround[i].searchlen = strlen(surround[i].search);

    for (i = 0; i < llen(replace); i++)
        replace[i].oldlen = strlen(replace[i].old);
    
    process(buf, buf + fz, 1);
    
    free(buf);

    return 0;
}
