import pandas as pd
import numpy as np
import random

def generate_student_data(num_students=100):
    data = []
    
    for _ in range(num_students):
        cgpa = round(random.uniform(5.0, 9.5), 2)  # CGPA between 5.0 and 9.5
        gpa = round(random.uniform(5.0, 9.8), 2)  # GPA between 5.0 and 9.8
        avg_assignment_marks = random.randint(10, 100)  # Assignment marks between 10 and 100
        avg_project_marks = random.randint(10, 100)  # Project marks between 10 and 100
        attendance = random.randint(50, 100)  # Attendance between 50% and 100%
        class_participation = random.randint(0, 100)  # Participation credits between 0 and 100
        extracurricular = random.randint(0, 1)  # 0 or 1
        achievements = random.randint(0, 100)  # Achievements credits between 0 and 100
        num_students_class = random.randint(30, 100)  # Class size between 30 and 100
        rank_in_class = random.randint(1, num_students_class)  # Rank between 1 and class size
        certifications = random.randint(0, 20)  # Certifications count between 0 and 20
        
        # Adjusted performance calculation to ensure it reaches 99
        performance = (
            (cgpa * 22) + (gpa * 18) + (avg_assignment_marks * 0.6) + (avg_project_marks * 0.6) + 
            (attendance * 0.35) + (class_participation * 2.2) + (extracurricular * 5) + 
            (achievements * 3.2) + ((num_students_class - rank_in_class) / num_students_class * 25) + 
            (certifications * 5)
        ) / 9  # Adjusted normalization factor

        # Ensure performance is between 30 and 99
        performance = max(30, min(99, round(performance, 2)))
        
        data.append([
            cgpa, gpa, avg_assignment_marks, avg_project_marks, attendance,
            class_participation, extracurricular, achievements,
            num_students_class, rank_in_class, certifications, performance
        ])
    
    columns = [
        "CGPA", "GPA", "Average Assignment Marks", "Average Project Marks", "Attendance Percentage",
        "Class Participation Credits", "Extracurricular Activities", "Achievements Credits",
        "No. of Students in Class", "Rank in Class", "Certifications Count", "Performance"
    ]
    
    df = pd.DataFrame(data, columns=columns)
    df.to_csv("student_performance2.csv", index=False)
    print("Dataset generated and saved as student_performance2.csv")

# Generate dataset
generate_student_data(30000) 

