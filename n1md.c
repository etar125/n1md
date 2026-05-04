/*
 * new etar125's markdown to html
 * Copyright (c) 2026 etar125 Admanse
 * Licensed under ISC license (see LICENSE)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <e1l.h>
#define ESTRNDUP
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
static int dolink(const char *begin, const char *end, int newblock);
static int dolist(const char *begin, const char *end, int newblock);
static int docode(const char *begin, const char *end, int newblock);
static int dohr(const char *begin, const char *end, int newblock);
static int doecmd(const char *begin, const char *end, int newblock);
static int doheader(const char *begin, const char *end, int newblock);

static void process(const char *begin, const char *end, int newblock);

static parser ps[] = { doecmd, docode, doheader, doprefix, dohr, dolist, doparagraph, donewline, dolink, dosurround, doreplace };

static rp replace[] = {
    { "\\\\",   "\\" },
    { "\\*",    "*" },
    { "\\`",    "`" },
    { "\\#",    "#" },

    { "\\&",     "&" },
    { "\\>",     ">" },
    { "\\<",     "<" },
    { "\\\"",    "\"" },

    { "&amp;", "&amp;" },
    { "&",     "&amp;" },
    { ">",     "&gt;" },
    { "<",     "&lt;" },
    { "\"",    "&quot;" },
};

static st prefix[] = {
    /* 
     * 0 - hprint
     * 1 - collect and hprint
     * 2 - collect and process as newblock
     */
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

char *class, *id, *classend, *idend;
int headerlink;

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
        else if (*p == '\\' && ++p < end)
            putc(*p, stdout);
        else
            putc(*p, stdout);
    }
}

int doheader(const char *begin, const char *end, int newblock) {
    const char *p, *q;
    int lvl;

    if (!newblock)
        return 0;

    p = begin;

    for (; p < end && *p == '#'; p++);
    if (p == begin || *p != ' ')
        return 0;
    lvl = p - begin;
    for (q = ++p; q < end && *q != '\n'; q++);
    if (lvl > 6)
        lvl = 6;
    
    printf("<h%d", lvl);
    if (id) {
        printf(" id=\"");
        printh(id, idend);
        printf("\"");
        printf(">");
        if (headerlink) {
            printf("<a class=\"header-link\" href=\"#");
            printh(id, idend);
            printf("\">");
        }
    } else { printf(">"); }
    printh(p, q);
    if (id) {
        free(id);
        id = NULL;
        if (headerlink)
            printf("</a>");
    }
    printf("</h%d>\n", lvl);

    return -(q - begin);
}

int doprefix(const char *begin, const char *end, int newblock) {
    const char *p, *q, *stop;
    char *ds, *rs;
    int i, c;
    size_t l, asize, bsize;
    
    if (!newblock)
        return 0;

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

int dolink(const char *begin, const char *end, int newblock) {
    const char *tb, *te, *lb, *le, *p;
    int img = 0, autolink = 0;

    p = begin;
    if (p + 1 < end && *p == '!' && *(p + 1) == '!') {
        p += 2;
        img = 1, autolink = 1;
    } else if (*p == '!' ) {
        p++;
        img = 1;
    }
    if (p + 3 >= end || *p != '[')
        return 0;
    tb = ++p;
    for (; p < end && *p != ']'; p++)
        if (*p == '\\')
            p++;
    if (p == end)
        return 0;
    te = p++;
    if (*p != '(')
        return 0;
    lb = ++p;
    for (; p < end && *p != ')'; p++)
        if (*p == '\\')
            p++;
    if (p == end)
        return 0;
    le = p;

    if (img) {
        if (autolink) {
            printf("<a href=\"");
            printh(lb, le);
            printf("\">");
        }
        printf("<img src=\"");
        printh(lb, le);
        printf("\" alt=\"");
        printh(tb, te);
        printf("\">");
        if (autolink)
            printf("</a>");
    } else {
        printf("<a href=\"");
        printh(lb, le);
        printf("\">");
        process(tb, te, 0);
        printf("</a>");
    }
    
    return le + 1 - begin;
}

int dolist(const char *begin, const char *end, int newblock) {
    const char *p, *q;
    char *ds = NULL, *rs = NULL, marker;
    int ol = 0, indent = 2, error = 0, ci = 0, nb = 0, task = 0;
    size_t asize = 0, bsize = 0;


    if (newblock)
        p = begin;
    else if (*begin == '\n')
        p = begin + 1;
    else
        return 0;
    
    q = p;
    if (*p == '-' || *p == '*' || *p == '+') {
        marker = *p;
    } else {
        for (q = p; p < end && isdigit(*p); p++);
        if (p == q || p == end || *p != '.')
            return 0;
        ol = 1, p = q;
    }
    p++;
    if (p >= end || *p != ' ')
        return 0;

    if (*begin == '\n') { puts(""); }
    puts(ol ? "<ol>" : "<ul>");

    for (p = q; p < end; p++) {
        q = p;
        
        if (ol) {
            for (; p < end && isdigit(*p); p++);
            if (p == q || p == end || *p != '.')
                break;
        } else if (*p != marker) {
            break;
        }
        p++;
        if (p >= end || *p != ' ')
            break;
        p++;

        if (ol)
            indent = p - q;
        nb = 0;

        task = 0;

        if (p + 2 < end) {
            if (*p == '[' && *(p + 2) == ']') {
                p++;
                if (*p == ' ')
                    task = 1;
                else if (isprint((unsigned char)(*p)))
                    task = 2;
                else
                    p -= 4;
                p += 2;
            }
        }

        for (; p < end; p++){
            if (*p == '\n') {
                if (p + 1 == end)
                    break;
                if (d_append(&ds, &asize, &bsize, "\n", 1) != 0) {
                    if (ds)
                        free(ds);
                    perror("d_append failed");
                    error = 1;
                    break;
                }
                for (ci = 0, q = p + 1; q < end && *q == ' ' && ci < indent; q++, ci++);
                if (ci < indent)
                    break;
                if (*q == '\n') {
                    nb = 1;
                }
                p = q - 1;
            } else {
                if (d_append(&ds, &asize, &bsize, p, 1) != 0) {
                    if (ds)
                        free(ds);
                    perror("d_append failed");
                    error = 1;
                    break;
                }
            }
        }

        if (error)
            break;

        rs = d_shrink(ds, asize, bsize);
        free(ds);
        ds = NULL, bsize = 0;
        if (!rs) {
            perror("d_shrink failed");
            break;
        }

        printf("<li>");

        if (task == 1)
            printf("<input type=\"checkbox\" disabled=\"\">");
        else if (task == 2)
            printf("<input type=\"checkbox\" checked=\"\" disabled=\"\">");

        process(rs, rs + asize, nb);
        asize = 0;
        free(rs);
        puts("</li>");
    }

    puts(ol ? "</ol>" : "</ul>");

    return p - begin - 1;
}

int docode(const char *begin, const char *end, int newblock) {
    const char *p, *q;

    if (!newblock || begin + 6 >= end || strncmp(begin, "```", 3) != 0)
        return 0;

    
    p = strstr(begin + 3, "\n```");
    if (!p || p > end)
        p = end;
    
    q = begin + 3;
    if (*q != '\n') {
        printf("<pre><code class=\"");
        for (; q < end && *q != '\n'; q++);
        printh(begin + 3, q);
        printf("\">");
    } else {
        puts("<pre><code>");
    }

    process(q + 1, p, 0);
    puts("</code></pre>");

    return -(p + 4 - begin);
}

int dohr(const char *begin, const char *end, int newblock) {
    const char *p = begin;
    int len;
    
    if (!newblock)
        return 0;

    for (; p < end && *p != '\n'; p++);

    len = p - begin;

    if ((len == 3 && strncmp(begin, "---", 3) == 0) ||
        (len == 5 && strncmp(begin, "- - -", 5) == 0))
        puts("<hr>");
    else
        return 0;

    return -len;
}

int doecmd(const char *begin, const char *end, int newblock) {
    const char *p, *q, *as = NULL;
    char *out = NULL;
    int len, ol, al, rlen;

    if (newblock)
        p = begin;
    else
        return 0;

    if (*p != '?' || ++p >= end)
        return 0;

    for (q = p; q < end && *q != '\n'; q++)
        if (!as && *q == ' ')
            as = q;
    len = q - p;
    rlen = q - begin;
    if (newblock) { rlen = -rlen; }
    if (!as || ++as >= q)
        return rlen;
    
    if (len > 6 && strncmp(p, "open", 4) == 0) {
        printf("<");
        printh(as, q);
        if (class) {
            printf(" class=\"");
            printh(class, classend);
            printf("\"");
            free(class);
            class = NULL;
        }
        if (id) {
            printf(" id=\"");
            printh(id, idend);
            printf("\"");
            free(id);
            id = NULL;
        }
        puts(">");
    } else if (len > 7 && strncmp(p, "close", 5) == 0) {
        printf("</");
        printh(as, q);
        puts(">");
    } else if (len > 7 && strncmp(p, "opt", 3) == 0) {
        p = as;
        for (; as < q && *as != ' '; as++);
        if (as + 1 >= q)
            return rlen;
        ol = as - p;
        as++;
        al = q - as;
        out = estrndup(as, al);
        if (!out)
            return rlen;

        if (ol == 5 && strncmp(p, "class", ol) == 0) {
            if (class)
                free(class);
            class = out;
            classend = out + al;
        } else if (ol == 2 && strncmp(p, "id", ol) == 0) {
            if (id)
                free(id);
            id = out;
            idend = out + al;
        } else if (ol == 2 && strncmp(p, "hl", ol) == 0) {
            if (*out == '1' || *out == 't' || *out == 'y')
                headerlink = 1;
            else
                headerlink = 0;
            free(out);
            out = NULL;
        } else { free(out); out = NULL; }
    } else if (len > 7 && strncmp(p, "unopt", 5) == 0) {
        ol = q - as;

        if (ol == 5 && strncmp(as, "class", ol) == 0 && class) {
            free(class);
            class = NULL;
        } else if (ol == 2 && strncmp(as, "id", ol) == 0 && id) {
            free(id);
            id = NULL;
        }
    } else if (len > 5 && strncmp(p, "com", 3) == 0) {
        printf("<!--");
        printh(as, q);
        puts("-->");
    }

    return rlen;
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
    class = NULL, id = NULL, headerlink = 0;

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
    if (class)
        free(class);
    if (id)
        free(id);

    return 0;
}
