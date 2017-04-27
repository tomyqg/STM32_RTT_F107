#ifndef __CHANNEL_H__ 
#define	__CHANNEL_H__

int process_channel(void);
int channel_need_change(void);
int getOldChannel(void);
int getNewChannel(void);
int saveECUChannel(int channel);
void changeChannelOfInverters(int oldChannel, int newChannel);

#endif /*__CHANNEL_H__*/
