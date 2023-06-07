typedef unsigned long long sid;

void init_logger();

void add_log(sid ses_id, int action);

sid get_id();

void save_log();