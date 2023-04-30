import 'dart:ffi';

import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';

void main() {
  runApp(Slicer());
}

class Slicer extends StatefulWidget {
  @override
  _SlicerState createState() => _SlicerState();
}

class _SlicerState extends State<Slicer> {
  String? filepath = " ";

  Future<String?> getFile() async {
    FilePickerResult? result = await FilePicker.platform.pickFiles();

    if(result != null) {
      String? path = result.files.single.path;
      if(path != null) {
        print(path);
        
        return path;
      } else {
        print("There was an error with the file selection!  path = null");
        return null;
      }
    } else {
      print("User cancelled the process");
      return null;
    }
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: "slicer",
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: Scaffold(
        body: Center(
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
                    print("path is $filepath");
                    setState(() {
                      filepath = path;
                    });
                  },
                  child: const Text("Select file"),
                ),
                Text('Selected file: $filepath'),
              ]
            ),
          ),
        )
      ),
    );
  }
}
