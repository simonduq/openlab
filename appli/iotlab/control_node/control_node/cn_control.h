#ifndef IOTLAB_CONTROL3_H_
#define IOTLAB_CONTROL3_H_

/** Start the control library */
void cn_control_start();

/** Add pre stop and post start commands */
void cn_control_config(void (*pre_stop_cmd)(), void (*post_start_cmd)());

/** Get the configured node id */
uint16_t cn_control_node_id(void);

#endif /* IOTLAB_CONTROL_H_ */
