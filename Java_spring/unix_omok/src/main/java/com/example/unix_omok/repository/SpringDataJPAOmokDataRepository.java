package com.example.unix_omok.repository;

import com.example.unix_omok.OmokData;
import org.springframework.data.jpa.repository.JpaRepository;

public interface SpringDataJPAOmokDataRepository extends JpaRepository<OmokData,Long>, OmokDataRepository {

}
