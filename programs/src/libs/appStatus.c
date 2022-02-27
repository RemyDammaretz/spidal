#include "../../include/appStatus.h"

void initStatus(status_t * status, int value) {
	status->status = value;
	pthread_mutex_init(&(status->mutex), NULL);
}

void setStatus(status_t * status, int value) {
	pthread_mutex_lock(&(status->mutex));
	status->status = value;
	pthread_mutex_unlock(&(status->mutex));
}

int getStatus(status_t * status) {
	int result;
	pthread_mutex_lock(&(status->mutex));
	result = status->status;
	pthread_mutex_unlock(&(status->mutex));
	return result;
}
