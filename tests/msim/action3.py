def run(ecl_state, schedule, report_step, summary_state, actionx_callback):

    if report_step == 1:
        schedule.shut_well("P1", report_step)
    elif report_step == 2:
        schedule.shut_well("P2", report_step)
    elif report_step == 3:
        schedule.shut_well("P3", report_step)
    elif report_step == 4:
        schedule.shut_well("P4", report_step)
    elif report_step == 5:
        schedule.open_well("P1", report_step)