package IO::Socket::Multicast;

require 5.005;
use strict;
use vars qw(@ISA @EXPORT_OK @EXPORT %EXPORT_TAGS $VERSION);

use IO::Socket;
use Carp 'croak';
require Exporter;
require DynaLoader;
eval <<END; # or warn "IO::Interface module not installed; Cannot use interface names.\n";
  use IO::Interface 0.94 'IFF_MULTICAST';
END

my @functions = qw(mcast_add mcast_drop mcast_if mcast_loopback 
		   mcast_ttl mcast_dest mcast_send);

%EXPORT_TAGS = ('all'       => \@functions,
		'functions' => \@functions);
@EXPORT = ( );
@EXPORT_OK = @{ $EXPORT_TAGS{'all'} };

@ISA = qw(Exporter DynaLoader);
$VERSION = '0.02';

my $IP = '\d+\.\d+\.\d+\.\d+';

# This "interesting" bit of code loads our functions into IO::Socket,
# to "improve" it.
{ 
  no strict 'refs';
  *{"IO\:\:Socket\:\:$_"} = \&$_ foreach @functions;
}

sub import {
  Socket->export_to_level(1,@_);
  IO::Socket::Multicast->export_to_level(1,@_);
}

sub mcast_add {
  my $sock = shift;
  my $group = shift || croak 'usage: $sock->mcast_add($mcast_addr [,$interface])';
  $group = inet_ntoa($group) unless $group =~ /^$IP$/o; 
  my $interface = get_if_addr($sock,shift);
  return $sock->_mcast_add($group,$interface);
}

sub mcast_drop {
  my $sock = shift;
  my $group = shift || croak 'usage: $sock->mcast_add($mcast_addr [,$interface])';
  $group = inet_ntoa($group) unless $group =~ /^$IP$/o; 
  my $interface = get_if_addr($sock,shift);
  return $sock->_mcast_drop($group,$interface);
}

sub mcast_if {
  my $sock = shift;

  my $previous = $sock->_mcast_if;
  $previous = $sock->addr_to_interface($previous) 
    if $sock->can('addr_to_interface');
  return $previous unless @_;

  my $interface = get_if_addr($sock,shift);
  return $sock->_mcast_if($interface) ? $previous : undef;
}

sub get_if_addr {
  my $sock = shift;
  return '0.0.0.0' unless defined (my $interface = shift);
  return $interface if $interface =~ /^$IP$/;
  return $interface if length $interface == 16;
  croak "IO::Interface module not available; use IP addr for interface"
    unless $sock->can('if_addr');
  croak "unknown or unconfigured interace $interface"
    unless my $addr = $sock->if_addr($interface);
  croak "interface is not multicast capable"
    unless $interface eq 'any' or ($sock->if_flags($interface) & IFF_MULTICAST());
  return $addr;
}

sub mcast_dest {
  my $sock = shift;
  my $prev = ${*$sock}{'io_socket_mcast_dest'};
  if (my $dest = shift) {
    $dest = sockaddr_in($2,inet_aton($1)) if $dest =~ /^($IP):(\d+)$/;
    croak "invalid destination address" unless length($dest) == 16;
    ${*$sock}{'io_socket_mcast_dest'} = $dest;
  }
  return $prev;
}

sub mcast_send {
  my $sock = shift;
  my $data = shift || croak 'usage: $sock->mcast_send($data [,$address])';
  $sock->mcast_dest(shift) if @_;
  my $dest = $sock->mcast_dest || croak "no destination specified with mcast_send() or mcast_dest()";
  return send($sock,$data,0,$dest);
}

bootstrap IO::Socket::Multicast $VERSION;

1;
__END__

=head1 NAME

IO::Socket::Multicast - Perl extension for blah blah blah

=head1 SYNOPSIS

  use IO::Socket::Multicast;

  # create a new UDP socket ready to read datagrams on port 1100
  my $s = IO::Socket::INET->new(Proto=>'udp',LocalPort=>1100);

  # Add a multicast group
  $s->mcast_add('225.0.1.1');

  # Add a multicast group to eth0 device
  $s->mcast_add('225.0.0.2','eth0');

  # now receive some multicast data
  $s->recv($data,1024);

  # Drop a multicast group
  $s->mcast_drop('225.0.0.1');

  # Set outgoing interface to eth0
  $s->mcast_if('eth0');

  # Set time to live on outgoing multicast packets
  $s->ttl(10);

  # Turn off loopbacking
  $s->loopback(0);

  # Multicast a message to group 225.0.0.1
  $s->mcast_send('hello world!','225.0.0.1:1200');
  $s->mcast_set('225.0.0.2:1200');
  $s->mcast_send('hello again!');

=head1 DESCRIPTION

The IO::Socket::Multicast module adds methods to IO::Socket that
enable you to manipulate multicast groups.  With this module (and an
operating system that supports multicasting), you will be able to
receive incoming multicast packets and generate your own outgoing
multicast packets.  This module is compatible with MBONE.

=head2 INTRODUCTION

Multicasting is designed for streaming multimedia applications and for
conferencing systems in which one transmitting machines needs to
distribute data to a large number of clients.

IP addresses in the range 224.0.0.0 and 239.255.255.255 are reserved
for multicasting.  These addresses do not correspond to individual
machines, but to multicast groups.  Messages sent to these addresses
will be delivered to a potentially large number of machines that have
registered their interest in receiving transmissions on these groups.
They work like TV channels.  A program tunes in to a multicast group
to receive transmissions to it, and tunes out when it no longer
wishes to receive the transmissions.

To receive transmissions B<from> a multicast group, you will use
IO::Socket::INET->new() to create a UDP socket and bind it to a local
network port.  You will then subscribe one or more multicast groups
using the mcast_add() method.  Subsequent calls to the standard recv()
method will now receive messages incoming messages transmitted to the
subscribed groups using the selected port number.

To send transmissions B<to> a multicast group, you can use the
standard send() method to send messages to the multicast group and
port of your choice.  The mcast_set() and mcast_send() methods are
provided as convenience functions.  Mcast_set() will set a default
multicast destination for messages which you then send with
mcast_send().

To set the number of hops (routers) that outgoing multicast messages
will cross, call mcast_ttl().  To activate or deactivate the looping
back of multicast messages (in which a copy of the transmitted
messages is received by the local machine), call mcast_loopback().

=head2 CONSTRUCTORS

=over 4

=item $socket = IO::Socket::INET->new(Proto=>'udp',LocalPort=>$port)

There is no special constructor for the IO::Socket::Multicast class.
Instead, loading the module adds methods to IO::Socket.  Create a UDP
socket as usual, using IO::Socket::INET.  Be sure to specify a
protocol of "udp" and, for sockets that will be used for incoming
multicast transmissions, to explicitly bind the socket to a port using
the B<LocalPort> argument.

=back

=head2 METHODS

=over 4

=item $success = $socket->mcast_add($multicast_address [,$interface])

The mcast_add() method will add the provided multicast address to the
list of subscribed multicast groups.  The address may be provided
either as a dotted-quad decimal, or as a packed IP address (such as
produced by the inet_aton() function).  On success, the method will
return a true value.

The optional $interface argument can be used to specify on which
network interface to listen for incoming multicast messages.  If the
IO::Interface module is installed, you may use the device name for the
interface (e.g. "tu0").  Otherwise, you must use the IP address of the
desired network interface.  Either dotted quad form or packed IP
address is acceptable.  If no interface is specified, then the
multicast group is joined on INADDR_ANY, meaning that multicast
transmissions received on B<any> of the host's network interfaces will
be forwarded to the socket.

=item $success = $socket->mcast_drop($multicast_address)

This reverses the action of mcast_add(), removing the indicated
multicast address from the list of subscribed groups.

=item $loopback = $socket->mcast_loopback

=item $previous = $socket->mcast_loopback($new)

The mcast_loopback() method controls whether the socket will receive
its own multicast transmissions (default yes).  Called without
arguments, the method returns the current state of the loopback
flag. Called with a boolean argument, the method will set the loopback
flag, and return its previous value.

=item $ttl = $socket->mcast_ttl

=item $previous = $socket->mcast_ttl($new)

The mcast_ttl() method examines or sets the time to live (TTL) for
outgoing multicast messages.  The TTL controls the numbers of routers
the packet can cross before being expired.  The default TTL is 1,
meaning that the message is confined to the local area network.
Values between 0 and 255 are valid.

Called without arguments, this method returns the socket's current
TTL.  Called with a value, this method sets the TTL and returns its
previous value.

=item $interface = $socket->mcast_if

=item $previous = $socket->mcast_if($new)

By default, the OS will pick the network interface to use for outgoing
multicasts automatically.  You can control this process by using the
mcast_if() method to set the outgoing network interface explicitly.
Called without arguments, returns the current interface.  Called with
the name of an interface, sets the outgoing interface and returns its
previous value.

You can use the device name for the interface (e.g. "tu0") if the
IO::Interface module is present.  Otherwise, you must use the
interface's dotted IP address.

B<NOTE>: To set the interface used for B<incoming> multicasts, use the
mcast_add() method.

=item $dest = $socket->mcast_dest

=item $previous = $socket->mcast_dest($new)

The mcast_dest() method is a convenience function that allows you to
set the default destination group for outgoing multicasts.  Called
without arguments, returns the current destination as a packed binary
sockaddr_in data structure.  Called with a new destination address,
the method sets the default destination and returns the previous one,
if any.

Destination addresses may be provided as packed sockaddr_in
structures, or in the form "XX.XX.XX.XX:YY" where the first part is
the IP address, and the second the port number.

=item $bytes = $socket->mcast_send($data [,$dest])

Mcast_send() is a convenience function that simplifies the sending of
multicast messages.  C<$data> is the message contents, and C<$dest> is
an optional destination group.  You can use either the dotted IP form
of the destination address and its port number, or a packed
sockaddr_in structure.  If the destination is not supplied, it will
default to the most recent value set in mcast_dest() or a previous
call to mcast_send().

The method returns the number of bytes successfully queued for
delivery.

As a side-effect, the method will call mcast_dest() to remember the
destination address.

Example:

  $socket->mcast_send('Hi there group members!','225.0.1.1:1900') || die;
  $socket->mcast_send("How's the weather?") || die;

Note that you may still call IO::Socket::INET->new() with a
B<PeerAddr>, and IO::Socket::INET will perform a connect(), creating a
default destination for calls to send().

=back

=head1 EXAMPLE

The following is an example of a multicast server.  Every 10 seconds
it transmits the current time and the list of logged-in users to the
local network using multicast group 226.1.1.2, port 2000 (these are
chosen arbitrarily).

 #!/usr/bin/perl
 # server
 use strict;
 use IO::Socket::Multicast;

 use constant DESTINATION => '226.1.1.2:2000'; 
 my $sock = IO::Socket::INET->new(Proto=>'udp',PeerAddr=>DESTINATION);

 while (1) {
   my $message = localtime;
   $message .= "\n" . `who`;
   $sock->send($message) || die "Couldn't send: $!";
 } continue {
   sleep 10;
 }

This is the corresponding client.  It listens for transmissions on
group 226.1.1.2, port 2000, and echoes the messages to standard
output.

 #!/usr/bin/perl
 # client

 use strict;
 use IO::Socket::Multicast;

 use constant GROUP => '226.1.1.2';
 use constant PORT  => '2000';

 my $sock = IO::Socket::INET->new(Proto=>'udp',LocalPort=>PORT);
 $sock->mcast_add(GROUP) || die "Couldn't set group: $!\n";

 while (1) {
   my $data;
   next unless $sock->recv($data,1024);
   print $data;
 }


=head2 EXPORT

None by default.  However, if you wish to call mcast_add(),
mcast_drop(), mcast_if(), mcast_loopback(), mcast_ttl, mcast_dest()
and mcast_send() as functions you may import them explicitly on the
B<use> line or by importing the tag ":functions".

=head1 AUTHOR

Lincoln Stein, lstein@cshl.org.

This module is distributed under the same terms as Perl itself.

=head1 SEE ALSO

perl(1), IO::Socket(3), IO::Socket::INET(3).

=cut
