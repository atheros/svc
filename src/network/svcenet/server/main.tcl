set COMMANDS [dict create]



# handle a newly connected peer
proc on_peer_connect {peer} {
	set nick "Unknown$peer"
	peerset pub $peer NICK			$peer
	peerset pub $peer CHANNEL		$DEFAULT_CHANNEL
	peerset pub $peer MUTED			0
	peerset pub $peer DEAFEN		0
	peerset pub $peer SPEAKING		0
	peerset pub $peer ADMIN			0
	
	
	puts "Peer #$peer $nick connected"
	return 1
}

# handle a peer disconnection
proc on_peer_disconnect {peer} {
	set nick [peerget pub $peer "NICK"]
	puts "Peer #$peer $nick disconnected"
}

# handle incoming audio packet
proc on_audio_packet {peer} {
	global SPEAK_TIME
	# don't send packet if source peer is muted
	if {[peerget pub $peer MUTED]} { return }
	# get all peers from a channel
	set peers [channel_peers [peerget pub $peer CHANNEL]]
	# set speaking on peer and speak stop timeout
	peerset priv $peer SPEAKING_TIMEOUT [expr [millitime] + $SPEAK_TIME]
	peerset pub $peer SPEAKING 1
	
	# for all the peers in the channel
	for p $peers {
		# if this is the source peer, ignore it
		if {$peer == $p} { continue }
		# if target peer is deafen
		if {[peerget pub $p DEAFEN]} { continue }
		
		# send current audio packet to peer
		send_audio_data $p
	}
}

# called from time to time, delta contains milliseconds since the last call
proc on_logic {delta} {
	set peers [peers_list]
	for p $peers {
		# mark peers as not speaking if timeout occurs
		if {[peerget pub $p SPEAKING] && [millitime] > [peerget priv $p SPEAKING_TIMEOUT]} {
			peerset pub $p SPEAKING 0
		}
	}
}

# handle command from client
proc on_command {peer, args} {
	global COMMANDS
	set command [lindex args 0]
	
	if {[dict exists $COMMANDS $command]} {
		[dict get $COMMANDS $command] $args
	} else {
		puts "Command '$command' from #$peer not found"
		sendmsg $peer "Command '$command' not found"
	}
}

# returns peers on a channel
proc channel_peers {channel} {
	set peers [peers_list]
	set cpeers [list]
	
	foreach peer $peers {
		if {[peerget pub $peer "CHANNEL"] == $channel} {
			lappend cpeers $peer
		}
	}

	return $cpeers	
}



