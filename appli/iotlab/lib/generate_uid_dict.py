#! /usr/bin/env python
# -*- coding: utf-8 -*-


""" Generate a C hash table that gives node number from uid """


import sys
import subprocess
import json
import textwrap

SOURCE_FILES_NAME = "iotlab_uid_num_hashtable"


def extract_json(json_str):
    """ Parse json input string to a python object
    >>> extract_json('{}')
    {}
    >>> extract_json('{ "a": 1, "b":[2]}')
    {u'a': 1, u'b': [2]}

    >>> import sys; sys.stderr = sys.stdout
    >>> extract_json('not json_string')
    Traceback (most recent call last):
    ...
    SystemExit: 1
    """
    try:
        answer_dict = json.loads(json_str)
    except ValueError:  # pragma: no cover
        sys.stderr.write('Could not load JSON object from input.\n')
        sys.exit(1)
    return answer_dict


def extract_nodes_uids(nodes_dict):
    """ Extract the experiment-cli nodes_dict and generate parsed uid dict """
    uids_dict = {}
    # format
    # {
    #      "at86rf231": {
    #             '<uid'>: [ <node_num>, node_num2]
    # ...
    # In correct cases, all uids will have only one node

    for node_dict in nodes_dict["items"]:
        uid = node_dict['uid']
        if uid == 'unknown':
            print node_dict["network_address"], uid
            continue  # invalid uid, unused node

        # uids are stored per radio_type
        _radio_type = node_dict["archi"].split(':')[1]
        radio_dict = uids_dict.setdefault(_radio_type, {})

        # Only one node with the same uid
        assert uid not in radio_dict, ("Non uniq uid: %r %r" %
                                       (uid, node_dict["network_address"]))

        # store node infos
        _type, _id = node_dict["network_address"].split('.')[0].split('-')
        radio_dict[uid] = (str(_type), int(_id))  # ('node_type', node_num)
    return uids_dict


def print_uids_and_nodes(nodes_uid_dict):
    """ Print the nodes_uid_dict """
    for radio_type, values in nodes_uid_dict.items():
        print "%s: " % radio_type
        print "\tlen: %u" % len(values)
        # for uid in sorted(values.keys()):
        #     print "\t%s : %r" % (uid, sorted(values[uid]))


def generate_hash_table(nodes_uid_dict):
    """ Generate a hash table dict """
    hash_table = {}

    for radio_type in nodes_uid_dict:
        radio_hash_table = hash_table.setdefault(radio_type, {})
        uids = nodes_uid_dict[radio_type]

        for uid, node in uids.iteritems():
            uid_hash = 0
            uid_int = int(uid, 16)
            uid_hash ^= 0xf & (uid_int >> 0)
            uid_hash ^= 0xf & (uid_int >> 4)
            uid_hash ^= 0xf & (uid_int >> 8)
            uid_hash ^= 0xf & (uid_int >> 12)

            uid_hash_table = radio_hash_table.setdefault(uid_hash, [])

            uid_hash_table.append((str(uid), tuple(node)))
            uid_hash_table.sort(key=(lambda x: int(x[0], 16)))

    return hash_table


def print_hash_table(radio_type, hash_table):
    """ Print the uids hash table for radio_type """
    print "Table for %r nodes" % radio_type
    table = hash_table[radio_type]
    for i in sorted(table.keys()):
        print "hash_table[%r]: len == %r" % (i, len(table[i]))
        # print "\t%r" % table[i]


def generate_a_c_hash_table(radio_type, hash_table, out_file_name):
    """ Generate a C hash table in 'out_file_name' .c and .h files """

    radio_hash_table = hash_table[radio_type]

    header_str = textwrap.dedent('''\
    /*
     * Generated from %s
     */

    #include <stdint.h>

    #define CC1101 0x1
    #define CC2420 0x2
    #define M3 0x3
    #define A8 0x8

    struct node_id_hash {
        uint16_t hash;
        uint32_t node;
    };

    struct node {
        uint8_t node_type;
        uint32_t node_num;
    };

    const struct node_id_hash * const nodes_uid_hash_table[16];


    struct node node_from_uid(uint16_t uid);
    ''' % __file__)

    with open(out_file_name + '.h', 'w') as header:
        header.write(header_str)

    body_str = textwrap.dedent('''\
    /*
     * Generated from %s
     */
    ''' % __file__)

    body_str += '#include "%s.h"\n\n' % out_file_name

    for num, nodes in radio_hash_table.iteritems():
        body_str += 'static const struct node_id_hash const '
        body_str += 'node_table_%x[] = {\n' % num

        # [('9076', ('m3', 208)), ('9472', ('m3', 51)), ('c370', ('m3', 289))]
        for node in nodes:
            hash_str, (node_type, node_id) = node

            node_value = "%s << 24 | %u" % (node_type.upper(), node_id)

            body_str += '    { 0x%s, %s },\n' % (hash_str, node_value)

        body_str += '    { 0, 0 },\n'
        body_str += '};\n\n'

    body_str += 'const struct node_id_hash * '
    body_str += 'const nodes_uid_hash_table[16] = {\n'

    for num in radio_hash_table:
        body_str += '    node_table_%x,\n' % num
    body_str += '};\n\n'

    with open(out_file_name + '.c', 'w') as source:
        source.write(body_str)


def _main():  # pragma: no cover
    """ Parse nodes list and print a uid hash table """

    json_str = subprocess.check_output('experiment-cli info -l', shell=True)
    json_dict = extract_json(json_str)

    nodes_uid_dict = extract_nodes_uids(json_dict)
    print_uids_and_nodes(nodes_uid_dict)

    hash_table = generate_hash_table(nodes_uid_dict)

    print ''
    print_hash_table('at86rf231', hash_table)

    generate_a_c_hash_table('at86rf231', hash_table, SOURCE_FILES_NAME)


if __name__ == '__main__':  # pragma: no cover
    _main()
