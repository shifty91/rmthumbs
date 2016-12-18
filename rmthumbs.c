#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>

#include "config.h"
#include "rmthumbs.h"

/*
 * module global variables
 */
static struct config conf;
static unsigned int cnt_deleted_files;

/* windows files, which have to be deleted! */
static const char *windows_files[] = {
	"Thumbs.db",
	"desktop.ini",
	NULL
};

/*
 * helper functions
 */
static void usage(void)
{
	fprintf(stderr, "usage: %s [-rv] <dir>  \n", PROGNAME);
	fprintf(stderr, "    -r: recursive                \n");
	fprintf(stderr, "    -v: verbose                  \n");
	fprintf(stderr, "%s version: %s\n", PROGNAME, VERSION);
	fprintf(stderr, "(C) 2015 %s <%s>  \n", AUTHOR, EMAIL);
}

static void *my_memcpy(char *restrict dst, const char *restrict src, size_t n)
{
    const char *ps = src;
    char *pd = dst;

    for (size_t i = 0; i < n; ++i)
        *pd++ = *ps++;

    return pd;
}

static void crawl(const char *path)
{
	struct stat probs;
	struct dirent *direntptr;

	if (lstat(path, &probs) == -1) {
		perror("lstat() failed");
		return;
	}

	/* file found */
	if (S_ISREG(probs.st_mode)) {
		for (int i = 0; windows_files[i]; ++i) {
			/* delete files, if necessary */
			if (!strcmp(basename((char *)path), windows_files[i])) {
				if (remove(path)) {
					perror("remove() failed");
					continue;
				}

				cnt_deleted_files++;
				if (conf.verbose)
					printf("deleted file: %s\n", path);
			}
		}

		return;
	}

	if (!conf.recursive || !S_ISDIR(probs.st_mode))
		return;

	/* work recursively */
	DIR *dirptr = opendir(path);

	if (!dirptr) {
		perror("opendir() failed");
		return;
	}

	while (errno = 0, (direntptr = readdir(dirptr)) != NULL) {
		char *name	   = direntptr->d_name, *p;
		size_t pathlen = strlen(path), namelen = strlen(name);
		char new_path[pathlen + namelen + 2]; /* + '/' + '\0' */

		/* jump over . and .. */
		if (!strcmp(name, ".") || !strcmp(name, ".."))
			continue;

		/* build new path */
		p = my_memcpy(new_path, path, pathlen);
		if (new_path[pathlen - 1] != '/')
			p = my_memcpy(p, "/", 1);

		*((char*)my_memcpy(p, name, namelen)) = '\0';

		/* recursively for new path */
		crawl(new_path);
	}

	if (errno != 0)
		perror("readdir() failed");

	if (closedir(dirptr) == -1)
		perror("closedir() failed");
}

int main(int argc, char *argv[])
{
	int c;

	/* parse arguments */
	while ((c = getopt(argc, argv, "rv")) != -1) {
		switch (c) {
		case 'r':
			conf.recursive = 1;
			break;
		case 'v':
			conf.verbose = 1;
			break;
		case '?':
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}

	if (argc - optind != 1) {
		usage();
		exit(EXIT_FAILURE);
	}

	/* go */
	crawl(argv[optind]);

	/* print result */
	printf("Deleted files: %d\n", cnt_deleted_files);

	return EXIT_SUCCESS;
}
