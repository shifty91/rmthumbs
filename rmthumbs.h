#ifndef __RMTHUMBS_H__
#define __RMTHUMBS_H__

#include <stdio.h>
#include <stdlib.h>

/* config parameters */
struct config {
	short recursive;
	short verbose;
};

/* printing */
#define pr_fmt "[%s %s:%d]: %s  " fmt "\n"
#define err(fmt, ...)													\
	do {																\
		fprintf(stderr, pr_fmt(fmt), "ERROR", __FILE__, __LINE__, ##__VA_ARGS__); \
		exit(EXIT_FAILURE);												\
	} while (0)

#endif
