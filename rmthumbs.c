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
static char *windows_files[] = {
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
			if (!strcmp(basename(path), windows_files[i])) {
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

	if (!conf.recursive)
		return;

	/* work recursively */
	if (S_ISDIR(probs.st_mode)) {
		DIR *dirptr = opendir(path);

		if (!dirptr) {
			perror("opendir failed()");
			return;
		}

		while (errno = 0, (direntptr = readdir(dirptr)) != NULL) {
			char *name     = direntptr->d_name;
			size_t pathlen = strlen(path), newlen;
			newlen = pathlen + strlen(name) + 2;  /* + '/' + '\0' */
			if (newlen < pathlen) {
				fprintf(stderr, "Too long path length detected!\n");
				continue;
			}
			char new_path[newlen];

			/* jump over . and .. */
			if (!strcmp(name, ".") || !strcmp(name, ".."))
				continue;

			/* build new path */
			strcpy(new_path, path);
			if (new_path[pathlen - 1] != '/')
				strcat(new_path, "/");

			strcat(new_path, name);

			/* recursively for new Path */
			crawl(new_path);
		}

		if (errno != 0)
			perror("readdir() failed");

		if (closedir(dirptr) == -1)
			perror("closedir() failed");
	}
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
