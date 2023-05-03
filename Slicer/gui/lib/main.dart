import 'dart:ffi';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';

void main() {
  runApp(const Slicer());
}

class Slicer extends StatelessWidget {
  const Slicer({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: "slicer",
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: SlicePage(),
    );
  }
}

class SlicePage extends StatefulWidget {
  @override
  State<SlicePage> createState() => _SlicePageState();
}

class _SlicePageState extends State<SlicePage> {
  String? filepath;
  final pathError = const SnackBar(content: Text("Error selecting the file."));

  Future<String?> getFile() async {
    FilePickerResult? result = await FilePicker.platform.pickFiles();

    if(result != null) {
      String? path = result.files.single.path;
      if(path != null) {
        debugPrint(path);
        return path;
      } else {
        debugPrint("There was an error with the file selection!  path = null");
        return null;
      }
    } else {
      debugPrint("User cancelled the process");
      return null;
    }
  }


  Future<dynamic> runAutotrace(String path) async {
    String exe = 'lib\\autotrace.exe';
    List<String> args = ['-centerline', path, '-o', '${path.replaceFirst(".bmp", "")}.svg'];
    Process process = await Process.start(exe, args);
    stdout.addStream(process.stdout);
    stderr.addStream(process.stderr);
    return await process.exitCode;
  }

  @override
  Widget build(BuildContext context) {
    return Material(
        child: Center(
          child: Padding(
            padding: const EdgeInsets.all(10.0),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                const Text('Image slicing'),
                const SizedBox(height: 5),
                const Text('Accepted format(s): *.bmp'),
                const SizedBox(height: 5),
                ElevatedButton(
                  onPressed: () async {
                    String? path = await getFile();
                    setState(() {
                      filepath = path;
                    });
                    debugPrint("path is $filepath");
                  },
                  child: const Text("Select file"),
                ),
                const SizedBox(height: 5),
                Text('Selected file: $filepath'),
                const SizedBox(height: 10),
                ElevatedButton(
                  onPressed: () {
                    if(filepath == null) {
                      debugPrint("SliceButton but Path is null, error");
                      showDialog(context: context, builder: (BuildContext context) {
                        return AlertDialog(
                          title: const Text("Error"),
                          content: const Text("There was an error selecting the file"),
                          actions: [
                            TextButton(
                              onPressed: () {
                                Navigator.of(context).pop();
                              },
                              child: Text("Ok"),
                            ),
                          ],
                        );
                       });
                    } else {
                      setState(() {
                          String temp = filepath as String;
                          runAutotrace(temp);
                      });
                    }
                  }, 
                  child: const Text("Slice!"),)
              ]
            ),
          ),
        )
      );
  }
}
