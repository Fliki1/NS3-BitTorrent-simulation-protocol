## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):

    obj = bld.create_ns3_program('internet-bittorrent',['bittorrent','stats','flow-monitor','network', 'internet', 'netanim', 'point-to-point', 'mobility', 'applications'])
    obj.source = 'internet-bittorrent.cc'

    obj = bld.create_ns3_program('bittorrent-p2p',['bittorrent','stats','flow-monitor','network', 'internet', 'netanim', 'point-to-point', 'mobility', 'applications'])
    obj.source = 'bittorrent-p2p.cc'
    
    obj = bld.create_ns3_program('bittorrent_clusterizzazione',['bittorrent','stats','flow-monitor','network', 'internet', 'netanim', 'point-to-point', 'mobility', 'applications'])
    obj.source = 'bittorrent_clusterizzazione.cc'

    obj = bld.create_ns3_program('vodsim', ['bittorrent'])

    if bld.env['ENABLE_REAL_TIME']:
        obj.source = 'vodsim.cc'
    else:
        obj.source = 'vodsim-no-realtime.cc'
