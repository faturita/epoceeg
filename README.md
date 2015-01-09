epoceeg
=======

EPOC Headset basic command line EEG Capture app.  Quick, Dirty, and useful !

Compile and do your work, and then:

  epoceeg [dat_filename] [subject] [duration in seconds].
  
* The [.dat] extension is better for log file.
* [Subject] will be a directory where the [.dat] file will be stored.
* Add the [duration in seconds] for the captured data.

The generated file will have 22 fields: packet counter, 14 EPOC EMotiv channel, GyroX, GyroY, timestamp, function id, function value, marker, Sync


Dependencies
============

* Windows 7
* EEG EPOC Emotiv Research Edition SDK for Windows (32 bits)


Important!
==========
Remember to compile the project on 32-bits.  Otherwise it won't work and you will have a lot of headaches.

Matlab
======
Addtionally, using the following function, you can import the data into Matlab:

-------------

    function output = loadepoceegrawbyfile(fullfile,dowemean)

	fid = fopen( fullfile );

	output_matrix = fscanf(fid, '%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f', [22 inf]);

	fclose(fid);

	output_matrix = output_matrix';
	output = output_matrix(:,2:15);

	if (dowemean == 1)
		[n,m]=size(output);
		output=output - ones(n,1)*mean(output,1);
	end
    end
    
------------

Authors
=======
Rodrigo Ramele @ CiC @ ITBA University, Argentina