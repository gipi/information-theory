#include<unistd.h>


/*
 * This function worries about to write all the count bytes
 * to the file descriptor unless some error occurres.
 *
 * On error the function returns the number of bytes really
 * written successfully (less than count). This behaviour
 * differs from the write(2) original behaviour.
 */
ssize_t xwrite(int fd, void* buf, size_t count) {
	/* If count is zero, xwrite() returns zero
	 * and has  no  other  results.*/
	if(!count)
		return count;
	
	size_t actually_count = 0, status = 0;

	while (actually_count < count) {
		status = write(fd, buf, count - actually_count);
		if (status < 0)
			break;

		actually_count += status;
	}

	return actually_count;
}
