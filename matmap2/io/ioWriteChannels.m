function result = ioWriteChannels(filename,Channels)

% function result = ioWriteChannels(filename,Channels)
%
% this function writes a fac-geometry file to file
%
% INPUT
%  filename - name of the output file
%  Channels - matrix containing the translation matrix for channel numbers and geometry
%     The first column matches the geometry node number and the second the 
%     the leadnumber in the tsdf data file.
%     Hence every row in the matrix marks a link between a geometry node and a 
%     measurement channel 
%
% OUTPUT
%  result   - is one in case of success or zero in case of failure
%
% SEE ioReadChannels

% JG Stinstra 2002

% Test whether Channels needs to be transposed

if size(Channels,1) == 1,
    lead = 1:length(Channels);
    geom = Channels;
    Channels = [lead' geom'];
end

if size(Channels,2) == 1,
    lead = 1:length(Channels);
    geom = Channels';
    Channels = [lead' geom'];
end    
    
if size(Channels,1) ~= 2,
    Channels = Channels';
    if size(Channels,1) ~= 2,
	msgError('Channels does not have proper dimensions',4);
        result = 0;
        return
    end
end

% First ensure that the file has the correct extension

[pn,fn,ext] = fileparts(filename);

% Try to correct a faulty file name
if strcmp(ext,'channels') == 0, % no match
    filename = fullfile(pn,[fn '.channels']);
end

FID = fopen(filename,'w');

if FID < 1,
    err = sprintf('Could not open file : %s\n',filename);
    msgError(err,3);
    result = 0;
    return
end

fprintf(FID,'%4d channels\n',size(Channels,2));  % write number of translation links in file
fprintf(FID,'% 5d %5d \n',Channels);
fclose(FID);

result = 1; % everything went OK
return
