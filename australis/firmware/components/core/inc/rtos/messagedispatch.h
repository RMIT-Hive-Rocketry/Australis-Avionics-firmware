// ALLOW FORMATTING
#ifndef MESSAGEDISPATCH_H
#define MESSAGEDISPATCH_H

#include <stdbool.h>
#include <stddef.h>

#include "topic.h"

bool queueComment(TopicAlias_t topic, void *data, size_t size);
bool queueArticle(TopicHandle_t topic, void *data, size_t size);

#endif
