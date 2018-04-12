function [lead,pts] = ioReadMs(filename)
% FUNCTION [label,pts] = ioReadMs(filename)
%
% DESCRIPTION
% This function disassembles a .ms file from the microscribe digitiser.
% It splits the label and the digitised positions into two arrays.
%
% INPUT
% filename    the name of the .ms file
%
% OUTPUT
% label       a cell array with all the labels recorded in the file
% pts         all the pts recorded in the file.
%
% SEE ALSO -

if ~exist(filename,'file'),
    [pn,fn,ext] = fileparts(filename);
    filename = fullfile(pn,[fn '.ms']);
    if ~exist(filename,'file'),
        error('Could not find file');
    end
end    

fid = fopen(filename,'r');

starline = 0;

% find out where data starts
lead = [];
pts = [];

while ~starline,
    line = fgetl(fid);
    if ~isempty(line),
        if line(1) == '*',
            starline = 1;
        end
    end    
end    

endoffile = 0;
while (~endoffile)&(~feof(fid)),
    line = fgetl(fid)
    if strncmp(line,'endoffile',9) == 1,
        endoffile = 1;
    else    
        index = findstr(line,sprintf('\t'));
        label = sscanf(line(1:index(1)-1),'%s');
        coord = sscanf(line(index(1):end),'%f  %f %f');
        if (~isempty(label))&(~isempty(coord)),
            lead{end+1} = label;
            pts(end+1,[1:3]) = coord';
        end
    end
end

fclose(fid);

pts = pts';         % just make it like my other pts files