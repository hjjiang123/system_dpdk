#include <iostream>
#include <curl/curl.h>
using namespace std;

/*
# get the list of all switches
# GET /stats/switches


# get group features stats of the switch
# GET /stats/groupfeatures/<dpid>
#
# get groups desc stats of the switch
# GET /stats/groupdesc/<dpid>[/<group_id>]
# Note: Specification of group id is optional (OpenFlow 1.5 or later)
#
# get groups stats of the switch
# GET /stats/group/<dpid>[/<group_id>]
# Note: Specification of group id is optional
*/
string get_url(string url);


/*
# Update the switch stats
#
# add a flow entry
# POST /stats/flowentry/add
#
# modify all matching flow entries
# POST /stats/flowentry/modify
#
# modify flow entry strictly matching wildcards and priority
# POST /stats/flowentry/modify_strict
#
# delete all matching flow entries
# POST /stats/flowentry/delete
#
# delete flow entry strictly matching wildcards and priority
# POST /stats/flowentry/delete_strict
#
# delete all flow entries of the switch
# DELETE /stats/flowentry/clear/<dpid>

# add a group entry
# POST /stats/groupentry/add
#
# modify a group entry
# POST /stats/groupentry/modify
#
# delete a group entry
# POST /stats/groupentry/delete
*/
string post_url(string url, string data);