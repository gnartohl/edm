.title		rpc_status_messages
.ident		'1-000'
.facility	RPC,104/PREFIX=RPC_

.severity	success
SUCCESS		<Normal completion>

.severity	error
NOMEM		<Insufficient virtual memory>
BADPARAM	<Bad function parameter>
ARGBUFOVFLO	<RPC argument buffer length exceeded>
INVTYPE		<Invalid RPC argument type>
DATAOVFLO	<RPC data buffer length exceeded>
BADSEQ		<Bad RPC sequence number>
BADFUNCINDEX	<Bad RPC function index>
NOSUCHFUNC	<Unknown RPC function>
NUMARGSINV	<Invalid number of arguments for RPC function>
