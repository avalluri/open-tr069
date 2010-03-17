// $Id$

#include "method.h"

const char *evcpe_method_type_to_str(enum evcpe_method_type type)
{
	switch(type) {
	case EVCPE_GET_RPC_METHODS:
		return "GetRPCMethods";
	case EVCPE_SET_PARAMETER_VALUES:
		return "SetParameterValues";
	case EVCPE_GET_PARAMETER_VALUES:
		return "GetParameterValues";
	case EVCPE_GET_PARAMETER_NAMES:
		return "GetParameterNames";
	case EVCPE_SET_PARAMETER_ATTRIBUTES:
		return "SetParameterAttributes";
	case EVCPE_GET_PARAMETER_ATTRIBUTES:
		return "GetParameterAttributes";
	case EVCPE_ADD_OBJECT:
		return "AddObject";
	case EVCPE_DELETE_OBJECT:
		return "DeleteObject";
	case EVCPE_REBOOT:
		return "Reboot";
	case EVCPE_DOWNLOAD:
		return "Download";
	case EVCPE_UPLOAD:
		return "Upload";
	case EVCPE_FACTORY_RESET:
		return "FactoryReset";
	case EVCPE_GET_QUEUED_TRANSFERS:
		return "GetQueuedTransfers";
	case EVCPE_GET_ALL_QUEUED_TRANSFERS:
		return "GetAllQueuedTransfers";
	case EVCPE_SCHEDULE_INFORM:
		return "ScheduleInform";
	case EVCPE_SET_VOUCHERS:
		return "SetVouchers";
	case EVCPE_GET_OPTIONS:
		return "GetOptions";
	case EVCPE_INFORM:
		return "Inform";
	case EVCPE_TRANSFER_COMPLETE:
		return "TransferComplete";
	case EVCPE_AUTONOMOUS_TRANSFER_COMPLETE:
		return "AutonomousTransferComplete";
	case EVCPE_REQUEST_DOWNLOAD:
		return "RequestDownload";
	case EVCPE_KICKED:
		return "Kicked";
	default:
		return "unknown";
	}
}
