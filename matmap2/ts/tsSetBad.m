function TSindices = tsSetBad(TSindices,lead)

% FUNCTION TSindices = tsSetBad(TSindices,lead)
%          TSdata    = tsSetBad(TSdata,lead)
%
% DESCRIPTION
% This function sets the bad lead markers. This function does not
% make a new copy of the data.
%
% INPUT
% TSindices        number of timeseries
% lead             number(s) of lead(s). 
%
% OUTPUT 
% -
% SEE ALSO tsSetBlank tsSetInterp

if isempty(lead), return; end
    
if isnumeric(TSindices),
    global TS;
    for p=TSindices,
        TS{p}.leadinfo(lead) = bitor(TS{p}.leadinfo(lead),1);
    end
end

if isstruct(TSindices),
    TSindices.leadinfo(lead) = bitor(TSindices.leadinfo(lead),1);
end

if iscell(TSindices),
    for p = 1:length(TSindices),
        TSindices{p}.leadinfo(lead) = bitor(TSindices{p}.leadinfo(lead),1);  
    end
end

return


